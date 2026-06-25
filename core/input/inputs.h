#pragma once

#include "key_codes.h"
#include <math/math_defs.h>

#include <bitset>

namespace feather {

class Input {
    static Input* _instance;

    std::bitset<static_cast<size_t>(Key::COUNT)> _held;
    std::bitset<static_cast<size_t>(Key::COUNT)> _just_pressed;
    std::bitset<static_cast<size_t>(Key::COUNT)> _just_released;

    std::bitset<static_cast<size_t>(MouseButton::COUNT)> _mouse_held;
    std::bitset<static_cast<size_t>(MouseButton::COUNT)> _mouse_just_pressed;
    std::bitset<static_cast<size_t>(MouseButton::COUNT)> _mouse_just_released;

    Vector2 _mouse_pos = Vector2::zero;
    Vector2 _mouse_delta = Vector2::zero;
    Vector2 _scroll_delta = Vector2::zero;

public:
    Input();

    static Input& get();

    bool is_key_pressed(Key key) const;
    bool is_key_held(Key key) const;
    bool is_key_released(Key key) const;

    bool is_mouse_button_pressed(MouseButton btn) const;
    bool is_mouse_button_held(MouseButton btn) const;
    bool is_mouse_button_released(MouseButton btn) const;

    // Position in window pixels
    Vector2 get_mouse_position() const;
    // Movement since last frame
    Vector2 get_mouse_delta() const;
    // Scroll wheel since last frame (x = horizontal, y = vertical)
    Vector2 get_scroll_delta() const;

    // Called by Window before polling events each frame
    void _begin_frame();
    // Called by Window for each raw SDL_Event* (passed as void* to hide SDL from header)
    void _process_event(const void* raw_event);
};

} // namespace feather
