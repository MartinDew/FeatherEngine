#include "inputs.h"

#include <framework/assert.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>

#include <optional>

namespace feather {

Input* Input::_instance = nullptr;

// ---- SDL scancode table (order must match Key enum) -------------------------

static constexpr SDL_Scancode KEY_SCANCODE_MAP[static_cast<size_t>(Key::COUNT)] = {
    // Letters A-Z
    SDL_SCANCODE_A, SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D,
    SDL_SCANCODE_E, SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H,
    SDL_SCANCODE_I, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L,
    SDL_SCANCODE_M, SDL_SCANCODE_N, SDL_SCANCODE_O, SDL_SCANCODE_P,
    SDL_SCANCODE_Q, SDL_SCANCODE_R, SDL_SCANCODE_S, SDL_SCANCODE_T,
    SDL_SCANCODE_U, SDL_SCANCODE_V, SDL_SCANCODE_W, SDL_SCANCODE_X,
    SDL_SCANCODE_Y, SDL_SCANCODE_Z,
    // Number row 0-9
    SDL_SCANCODE_0, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
    SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7,
    SDL_SCANCODE_8, SDL_SCANCODE_9,
    // Numpad
    SDL_SCANCODE_KP_0, SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3,
    SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_7,
    SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_9,
    SDL_SCANCODE_KP_PLUS, SDL_SCANCODE_KP_MINUS, SDL_SCANCODE_KP_MULTIPLY,
    SDL_SCANCODE_KP_DIVIDE, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_KP_PERIOD,
    // Function F1-F12
    SDL_SCANCODE_F1,  SDL_SCANCODE_F2,  SDL_SCANCODE_F3,  SDL_SCANCODE_F4,
    SDL_SCANCODE_F5,  SDL_SCANCODE_F6,  SDL_SCANCODE_F7,  SDL_SCANCODE_F8,
    SDL_SCANCODE_F9,  SDL_SCANCODE_F10, SDL_SCANCODE_F11, SDL_SCANCODE_F12,
    // Arrows
    SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN,
    // Navigation
    SDL_SCANCODE_HOME, SDL_SCANCODE_END, SDL_SCANCODE_PAGEUP, SDL_SCANCODE_PAGEDOWN,
    SDL_SCANCODE_INSERT, SDL_SCANCODE_DELETE,
    // Special
    SDL_SCANCODE_SPACE, SDL_SCANCODE_RETURN, SDL_SCANCODE_ESCAPE,
    SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_TAB,
    // Modifiers
    SDL_SCANCODE_LSHIFT, SDL_SCANCODE_RSHIFT,
    SDL_SCANCODE_LCTRL,  SDL_SCANCODE_RCTRL,
    SDL_SCANCODE_LALT,   SDL_SCANCODE_RALT,
    SDL_SCANCODE_LGUI,   SDL_SCANCODE_RGUI,
    // Punctuation / symbols
    SDL_SCANCODE_COMMA,        SDL_SCANCODE_PERIOD,     SDL_SCANCODE_SLASH,
    SDL_SCANCODE_BACKSLASH,    SDL_SCANCODE_SEMICOLON,  SDL_SCANCODE_APOSTROPHE,
    SDL_SCANCODE_LEFTBRACKET,  SDL_SCANCODE_RIGHTBRACKET,
    SDL_SCANCODE_MINUS,        SDL_SCANCODE_EQUALS,     SDL_SCANCODE_GRAVE,
    // Misc
    SDL_SCANCODE_CAPSLOCK, SDL_SCANCODE_NUMLOCKCLEAR, SDL_SCANCODE_SCROLLLOCK,
    SDL_SCANCODE_PRINTSCREEN, SDL_SCANCODE_PAUSE,
};

// ---- Helpers -----------------------------------------------------------------

static std::optional<Key> scancode_to_key(SDL_Scancode sc) {
    for (size_t i = 0; i < static_cast<size_t>(Key::COUNT); ++i) {
        if (KEY_SCANCODE_MAP[i] == sc)
            return static_cast<Key>(i);
    }
    return std::nullopt;
}

static std::optional<MouseButton> sdl_button_to_mouse_button(uint8_t btn) {
    switch (btn) {
    case SDL_BUTTON_LEFT:   return MouseButton::Left;
    case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
    case SDL_BUTTON_RIGHT:  return MouseButton::Right;
    case SDL_BUTTON_X1:     return MouseButton::X1;
    case SDL_BUTTON_X2:     return MouseButton::X2;
    default:                return std::nullopt;
    }
}

// ---- Input -------------------------------------------------------------------

Input::Input() {
    fassert(!_instance);
    _instance = this;
}

Input& Input::get() {
    fassert(_instance);
    return *_instance;
}

void Input::_begin_frame() {
    _just_pressed.reset();
    _just_released.reset();
    _mouse_just_pressed.reset();
    _mouse_just_released.reset();
    _mouse_delta  = Vector2::zero;
    _scroll_delta = Vector2::zero;
}

void Input::_process_event(const void* raw_event) {
    const auto& event = *static_cast<const SDL_Event*>(raw_event);

    switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
        if (event.key.repeat)
            break;
        auto key = scancode_to_key(event.key.scancode);
        if (!key)
            break;
        auto idx = static_cast<size_t>(*key);
        _held.set(idx);
        _just_pressed.set(idx);
        break;
    }
    case SDL_EVENT_KEY_UP: {
        auto key = scancode_to_key(event.key.scancode);
        if (!key)
            break;
        auto idx = static_cast<size_t>(*key);
        _held.reset(idx);
        _just_released.set(idx);
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        auto btn = sdl_button_to_mouse_button(event.button.button);
        if (!btn)
            break;
        auto idx = static_cast<size_t>(*btn);
        _mouse_held.set(idx);
        _mouse_just_pressed.set(idx);
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        auto btn = sdl_button_to_mouse_button(event.button.button);
        if (!btn)
            break;
        auto idx = static_cast<size_t>(*btn);
        _mouse_held.reset(idx);
        _mouse_just_released.set(idx);
        break;
    }
    case SDL_EVENT_MOUSE_MOTION:
        _mouse_pos    = Vector2(event.motion.x, event.motion.y);
        _mouse_delta += Vector2(event.motion.xrel, event.motion.yrel);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        _scroll_delta += Vector2(event.wheel.x, event.wheel.y);
        break;
    default:
        break;
    }
}

// ---- Keyboard queries --------------------------------------------------------

bool Input::is_key_pressed(Key key) const  { return _just_pressed[static_cast<size_t>(key)]; }
bool Input::is_key_held(Key key) const     { return _held[static_cast<size_t>(key)]; }
bool Input::is_key_released(Key key) const { return _just_released[static_cast<size_t>(key)]; }

// ---- Mouse queries -----------------------------------------------------------

bool Input::is_mouse_button_pressed(MouseButton btn) const  { return _mouse_just_pressed[static_cast<size_t>(btn)]; }
bool Input::is_mouse_button_held(MouseButton btn) const     { return _mouse_held[static_cast<size_t>(btn)]; }
bool Input::is_mouse_button_released(MouseButton btn) const { return _mouse_just_released[static_cast<size_t>(btn)]; }

Vector2 Input::get_mouse_position() const { return _mouse_pos; }
Vector2 Input::get_mouse_delta() const    { return _mouse_delta; }
Vector2 Input::get_scroll_delta() const   { return _scroll_delta; }

} // namespace feather
