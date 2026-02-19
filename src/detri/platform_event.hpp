#pragma once

#include <cstdint>
#include <variant>

namespace detri
{
    enum class key : uint32_t
    {
        escape, enter, tab, backspace, space, left, right, up, down, page_up, page_down, home, end, insert, delete_, a, b, c,
        d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z, zero, one, two, three, four, five, six,
        seven, eight, nine, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, left_shift, left_control, left_alt,
        left_system, right_shift, right_control, right_alt, right_system, unknown
    };

    enum class mouse_button : uint32_t
    {
        left, right, middle, x1, x2, unknown
    };

    struct key_event {
        key value {key::unknown};
        bool pressed {};
        bool repeated {};
    };

    struct mouse_button_event {
        mouse_button button {mouse_button::left};
        bool pressed {};
        std::int32_t x {};
        std::int32_t y {};
    };

    struct mouse_move_event {
        std::int32_t x {};
        std::int32_t y {};
    };

    struct mouse_delta_event {
        std::int32_t dx {};
        std::int32_t dy {};
    };

    struct close_event {};
    struct resize_event
    {
        uint32_t width{};
        uint32_t height{};
    };
    struct resize_begin_event {};
    struct resize_end_event {};

    using event = std::variant<
        close_event,
        resize_event,
        resize_begin_event,
        resize_end_event,
        key_event,
        mouse_button_event,
        mouse_move_event,
        mouse_delta_event>;
}
