#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "detri/platform_event.hpp"
#include "detri/platform.hpp"

namespace detri
{
    using native_message_hook = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);

    enum class cursor_mode
    {
        normal, captured_hidden
    };

    struct window_size
    {
        uint32_t width {};
        uint32_t height {};
    };

    class window
    {
    public:
        static window create(const std::string& title, uint32_t width, uint32_t height);

        window() = delete;

        ~window();

        window(window&&) noexcept;

        window& operator=(window&&) noexcept;

        [[nodiscard]] bool is_open() const noexcept;

        void request_close() const noexcept;

        void show() const noexcept;

        void pump_messages();

        std::optional<event> poll_event();

        [[nodiscard]] window_size size() const noexcept;

        void set_cursor_mode(cursor_mode mode) const;

        [[nodiscard]] cursor_mode get_cursor_mode() const noexcept;

        [[nodiscard]] void* native_handle() const noexcept;

#ifdef _WIN32
        struct native_win32_handle
        {
            HWND hwnd;
            HINSTANCE hinstance;
        };
        [[nodiscard]] native_win32_handle native_win32() const noexcept;
#endif

    private:
        struct impl;

        explicit window(std::unique_ptr<impl>&& impl) noexcept;

        std::unique_ptr<impl> m_impl;
    };
}
