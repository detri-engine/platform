#include "detri/window.hpp"
#include "detri/platform_exceptions.hpp"

#include <atomic>
#include <expected>
#include <optional>
#include <queue>

namespace detri
{
    namespace
    {
        std::atomic<native_message_hook> g_native_message_hook{nullptr};
    }

    void set_native_message_hook(native_message_hook hook) noexcept
    {
        g_native_message_hook.store(hook, std::memory_order_release);
    }

    LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    namespace
    {
        constexpr auto window_class_name = L"platform.window";

        key map_key(const WPARAM wparam) noexcept
        {
            switch (wparam)
            {
                case VK_ESCAPE:
                    return key::escape;
                case VK_RETURN:
                    return key::enter;
                case VK_TAB:
                    return key::tab;
                case VK_BACK:
                    return key::backspace;
                case VK_SPACE:
                    return key::space;
                case VK_LEFT:
                    return key::left;
                case VK_RIGHT:
                    return key::right;
                case VK_UP:
                    return key::up;
                case VK_DOWN:
                    return key::down;
                case VK_LCONTROL:
                    return key::left_control;
                case VK_RCONTROL:
                    return key::right_control;
                case VK_CONTROL:
                    return key::left_control;
                case VK_LSHIFT:
                    return key::left_shift;
                case VK_RSHIFT:
                    return key::right_shift;
                case 'A':
                    return key::a;
                case 'B':
                    return key::b;
                case 'C':
                    return key::c;
                case 'D':
                    return key::d;
                case 'E':
                    return key::e;
                case 'F':
                    return key::f;
                case 'G':
                    return key::g;
                case 'H':
                    return key::h;
                case 'I':
                    return key::i;
                case 'J':
                    return key::j;
                case 'K':
                    return key::k;
                case 'L':
                    return key::l;
                case 'M':
                    return key::m;
                case 'N':
                    return key::n;
                case 'O':
                    return key::o;
                case 'P':
                    return key::p;
                case 'Q':
                    return key::q;
                case 'R':
                    return key::r;
                case 'S':
                    return key::s;
                case 'T':
                    return key::t;
                case 'U':
                    return key::u;
                case 'V':
                    return key::v;
                case 'W':
                    return key::w;
                case 'X':
                    return key::x;
                case 'Y':
                    return key::y;
                case 'Z':
                    return key::z;
                case '0':
                    return key::zero;
                case '1':
                    return key::one;
                case '2':
                    return key::two;
                case '3':
                    return key::three;
                case '4':
                    return key::four;
                case '5':
                    return key::five;
                case '6':
                    return key::six;
                case '7':
                    return key::seven;
                case '8':
                    return key::eight;
                case '9':
                    return key::nine;
                case VK_F1:
                    return key::f1;
                case VK_F2:
                    return key::f2;
                case VK_F3:
                    return key::f3;
                case VK_F4:
                    return key::f4;
                case VK_F5:
                    return key::f5;
                case VK_F6:
                    return key::f6;
                case VK_F7:
                    return key::f7;
                case VK_F8:
                    return key::f8;
                case VK_F9:
                    return key::f9;
                case VK_F10:
                    return key::f10;
                case VK_F11:
                    return key::f11;
                case VK_F12:
                    return key::f12;
                default:
                    return key::unknown;
            }
        }

        mouse_button map_mouse_button(const UINT message, const WPARAM wparam) noexcept
        {
            switch (message)
            {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    return mouse_button::left;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    return mouse_button::right;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    return mouse_button::middle;
                case WM_XBUTTONDOWN:
                case WM_XBUTTONUP:
                    return GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? mouse_button::x1 : mouse_button::x2;
                default:
                    return mouse_button::unknown;
            }
        }

        void register_window_class(HINSTANCE instance)
        {
            static ATOM atom = 0;
            if (atom != 0)
            {
                return;
            }

            WNDCLASSEXW window_class{};
            window_class.cbSize = sizeof(WNDCLASSEXW);
            window_class.style = CS_HREDRAW | CS_VREDRAW;
            window_class.lpfnWndProc = window_proc;
            window_class.cbClsExtra = 0;
            window_class.cbWndExtra = 0;
            window_class.hInstance = instance;
            window_class.hIcon = nullptr;
            window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            window_class.hbrBackground = nullptr;
            window_class.lpszMenuName = nullptr;
            window_class.lpszClassName = window_class_name;
            window_class.hIconSm = nullptr;

            atom = RegisterClassExW(&window_class);
            if (atom == 0)
            {
                const auto error = GetLastError();
                if (error != ERROR_CLASS_ALREADY_EXISTS)
                {
                    throw except::window_error{"Failed to register window class. Windows error code: " + std::to_string(error)};
                }

                atom = 1;
            }
        }

        struct window_state
        {
            HWND hwnd{};
            HINSTANCE instance{};
            bool is_open{true};
            cursor_mode cursor{cursor_mode::normal};
            bool suppress_next_mouse_move{false};
            std::queue<event> events;
        };
    } // namespace

    struct window::impl
    {
        std::unique_ptr<window_state> state{std::make_unique<window_state>()};
    };

    window::window(std::unique_ptr<impl>&& impl) noexcept
        : m_impl(std::move(impl))
    {
    }

    LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        auto* state = reinterpret_cast<window_state*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        if (message == WM_NCCREATE)
        {
            const auto* create_struct = reinterpret_cast<CREATESTRUCTW*>(lparam);
            state = static_cast<window_state*>(create_struct->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            state->hwnd = hwnd;
            return DefWindowProcW(hwnd, message, wparam, lparam);
        }

        if (state == nullptr)
        {
            return DefWindowProcW(hwnd, message, wparam, lparam);
        }

        if (const auto hook = g_native_message_hook.load(std::memory_order_acquire); hook != nullptr)
        {
            (void)hook(
                hwnd,
                static_cast<std::uint32_t>(message),
                static_cast<std::uintptr_t>(wparam),
                static_cast<std::intptr_t>(lparam));
        }

        switch (message)
        {
            case WM_CLOSE:
                state->is_open = false;
                state->events.emplace(close_event{});
                DestroyWindow(hwnd);
                return 0;
            case WM_DESTROY:
                state->is_open = false;
                return 0;
            case WM_SIZE:
                state->events.emplace(resize_event{
                    .width = static_cast<std::uint32_t>(LOWORD(lparam)),
                    .height = static_cast<std::uint32_t>(HIWORD(lparam))
                });
                return 0;
            case WM_ENTERSIZEMOVE:
                state->events.emplace(resize_begin_event{});
                return 0;
            case WM_EXITSIZEMOVE:
                state->events.emplace(resize_end_event{});
                return 0;
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                state->events.emplace(key_event{
                    .value = map_key(wparam),
                    .pressed = true,
                    .repeated = (lparam & (1LL << 30)) != 0
                });
                return 0;
            case WM_KEYUP:
            case WM_SYSKEYUP:
                state->events.emplace(key_event{
                    .value = map_key(wparam),
                    .pressed = false,
                    .repeated = false
                });
                return 0;
            case WM_MOUSEMOVE:
            {
                const std::int32_t x = GET_X_LPARAM(lparam);
                const std::int32_t y = GET_Y_LPARAM(lparam);
                state->events.emplace(mouse_move_event{
                    .x = x,
                    .y = y
                });
                if (state->cursor == cursor_mode::captured_hidden)
                {
                    if (state->suppress_next_mouse_move)
                    {
                        state->suppress_next_mouse_move = false;
                        return 0;
                    }

                    RECT client{};
                    if (GetClientRect(hwnd, &client) != 0)
                    {
                        const std::int32_t center_x = (client.right - client.left) / 2;
                        const std::int32_t center_y = (client.bottom - client.top) / 2;
                        const std::int32_t dx = x - center_x;
                        const std::int32_t dy = y - center_y;
                        if (dx != 0 || dy != 0)
                        {
                            state->events.emplace(mouse_delta_event{
                                .dx = dx,
                                .dy = dy
                            });
                            POINT center{
                                .x = center_x,
                                .y = center_y
                            };
                            ClientToScreen(hwnd, &center);
                            state->suppress_next_mouse_move = true;
                            SetCursorPos(center.x, center.y);
                        }
                    }
                }
                return 0;
            }
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
                state->events.emplace(mouse_button_event{
                    .button = map_mouse_button(message, wparam),
                    .pressed = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN ||
                               message == WM_XBUTTONDOWN,
                    .x = GET_X_LPARAM(lparam),
                    .y = GET_Y_LPARAM(lparam)
                });
                return 0;
            default:
                return DefWindowProcW(hwnd, message, wparam, lparam);
        }
    }

    window window::create(const std::string& title, const std::uint32_t width, const std::uint32_t height)
    {
        const auto wide_title = to_wstring(title);
        if (width == 0 || height == 0)
        {
            throw except::window_error{"Window dimensions must be greater than zero."};
        }

        auto impl = std::make_unique<window::impl>();
        HINSTANCE instance = GetModuleHandleW(nullptr);
        register_window_class(instance);
        impl->state->instance = instance;

        RECT rectangle{};
        rectangle.left = 0;
        rectangle.top = 0;
        rectangle.right = static_cast<LONG>(width);
        rectangle.bottom = static_cast<LONG>(height);
        AdjustWindowRectEx(&rectangle, WS_OVERLAPPEDWINDOW, FALSE, 0);

        impl->state->hwnd = CreateWindowExW(
            0,
            window_class_name,
            wide_title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rectangle.right - rectangle.left,
            rectangle.bottom - rectangle.top,
            nullptr,
            nullptr,
            instance,
            impl->state.get()
        );

        if (impl->state->hwnd == nullptr)
        {
            throw except::window_error{"Failed to create window. Windows error code: " + std::to_string(GetLastError())};
        }

        return window{std::move(impl)};
    }

    std::wstring to_wstring(const std::string& str)
    {
        if (str.empty())
        {
            return {};
        }
        std::wstring wstr;

        const int required_size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);

        if (required_size == 0)
        {
            throw except::string_conversion_error{"Couldn't convert UTF-8 to UTF-16. Windows error code: " + std::to_string(GetLastError())};
        }

        wstr.resize(required_size);

        int converted_size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], required_size);

        if (converted_size == 0)
        {
            throw except::string_conversion_error{"Couldn't convert UTF-8 to UTF-16. Windows error code: " + std::to_string(GetLastError())};
        }

        return wstr;
    }

    window::~window()
    {
        if (m_impl != nullptr && m_impl->state != nullptr && m_impl->state->hwnd != nullptr && IsWindow(m_impl->state->hwnd)
            != 0)
        {
            if (m_impl->state->cursor == cursor_mode::captured_hidden)
            {
                set_cursor_mode(cursor_mode::normal);
            }
            DestroyWindow(m_impl->state->hwnd);
        }
    }

    window::window(window&&) noexcept = default;

    window& window::operator=(window&&) noexcept = default;

    bool window::is_open() const noexcept
    {
        return m_impl != nullptr && m_impl->state != nullptr && m_impl->state->is_open;
    }

    void window::request_close() const noexcept
    {
        if (m_impl != nullptr && m_impl->state != nullptr && m_impl->state->hwnd != nullptr && IsWindow(m_impl->state->hwnd)
            != 0)
        {
            PostMessageW(m_impl->state->hwnd, WM_CLOSE, 0, 0);
        }
    }

    void window::show() const noexcept
    {
        if (m_impl != nullptr && m_impl->state != nullptr && m_impl->state->hwnd != nullptr)
        {
            ShowWindow(m_impl->state->hwnd, SW_SHOW);
            UpdateWindow(m_impl->state->hwnd);
        }
    }

    void window::pump_messages()
    {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0)
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    std::optional<event> window::poll_event()
    {
        pump_messages();
        if (m_impl == nullptr || m_impl->state == nullptr || m_impl->state->events.empty())
        {
            return std::nullopt;
        }

        event next_event = m_impl->state->events.front();
        m_impl->state->events.pop();
        return next_event;
    }

    window_size window::size() const noexcept
    {
        if (m_impl == nullptr || m_impl->state == nullptr || m_impl->state->hwnd == nullptr)
        {
            return {};
        }

        RECT client_rect{};
        if (GetClientRect(m_impl->state->hwnd, &client_rect) == 0)
        {
            return {};
        }

        const auto width = client_rect.right - client_rect.left;
        const auto height = client_rect.bottom - client_rect.top;
        return {
            .width = width > 0 ? static_cast<std::uint32_t>(width) : 0U,
            .height = height > 0 ? static_cast<std::uint32_t>(height) : 0U
        };
    }

    void window::set_cursor_mode(const cursor_mode mode) const
    {
        if (m_impl == nullptr || m_impl->state == nullptr || m_impl->state->hwnd == nullptr)
        {
            throw except::window_error{"Cannot set cursor mode on an invalid window."};
        }
        if (m_impl->state->cursor == mode)
        {
            return;
        }

        m_impl->state->cursor = mode;
        HWND hwnd = m_impl->state->hwnd;

        if (mode == cursor_mode::captured_hidden)
        {
            while (ShowCursor(FALSE) >= 0)
            {
            }

            RECT client{};
            if (GetClientRect(hwnd, &client) == 0)
            {
                throw except::window_error{"GetClientRect failed while enabling captured cursor mode."};
            }
            POINT tl{client.left, client.top};
            POINT br{client.right, client.bottom};
            ClientToScreen(hwnd, &tl);
            ClientToScreen(hwnd, &br);
            RECT clip_rect{tl.x, tl.y, br.x, br.y};
            if (ClipCursor(&clip_rect) == 0)
            {
                throw except::window_error{"ClipCursor failed while enabling captured cursor mode."};
            }

            const std::int32_t center_x = (client.right - client.left) / 2;
            const std::int32_t center_y = (client.bottom - client.top) / 2;
            POINT center{
                .x = center_x,
                .y = center_y
            };
            ClientToScreen(hwnd, &center);
            m_impl->state->suppress_next_mouse_move = true;
            SetCursorPos(center.x, center.y);
        }
        else
        {
            ClipCursor(nullptr);
            while (ShowCursor(TRUE) < 0)
            {
            }
            m_impl->state->suppress_next_mouse_move = false;
        }
    }

    cursor_mode window::get_cursor_mode() const noexcept
    {
        if (m_impl == nullptr || m_impl->state == nullptr)
        {
            return cursor_mode::normal;
        }
        return m_impl->state->cursor;
    }

    void* window::native_handle() const noexcept
    {
        if (m_impl == nullptr)
        {
            return nullptr;
        }

        return m_impl->state == nullptr ? nullptr : m_impl->state->hwnd;
    }

#if defined(_WIN32)
    window::native_win32_handle window::native_win32() const noexcept
    {
        if (m_impl == nullptr || m_impl->state == nullptr)
        {
            return {};
        }

        return {
            .hwnd = m_impl->state->hwnd,
            .hinstance = m_impl->state->instance
        };
    }
#endif
} // namespace platform
