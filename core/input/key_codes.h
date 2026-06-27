#pragma once

#include <cstdint>

namespace feather {

enum class Key : uint32_t {
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Number row
    Row0, Row1, Row2, Row3, Row4, Row5, Row6, Row7, Row8, Row9,
    // Numpad
    Kp0, Kp1, Kp2, Kp3, Kp4, Kp5, Kp6, Kp7, Kp8, Kp9,
    KpPlus, KpMinus, KpMultiply, KpDivide, KpEnter, KpDecimal,
    // Function
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // Arrows
    Left, Right, Up, Down,
    // Navigation
    Home, End, PageUp, PageDown,
    Insert, Delete,
    // Special
    Space, Enter, Escape, Backspace, Tab,
    // Modifiers
    LeftShift, RightShift,
    LeftCtrl, RightCtrl,
    LeftAlt, RightAlt,
    LeftSuper, RightSuper,
    // Punctuation / symbols
    Comma, Period, Slash, Backslash,
    Semicolon, Apostrophe,
    LeftBracket, RightBracket,
    Minus, Equals, Grave,
    // Misc
    CapsLock, NumLock, ScrollLock,
    PrintScreen, Pause,
    COUNT
};

enum class MouseButton : uint32_t {
    Left,
    Middle,
    Right,
    X1,
    X2,
    COUNT
};

} // namespace feather
