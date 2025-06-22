#pragma once

namespace CE
{
    ENUM()
    enum class KeyCode : s32
    {
        Unknown = 0,
        Return = '\r',
        Escape = '\x1B',
        Backspace = '\b',
        Tab = '\t',
        Space = ' ',
        Exclaim = '!',
        DoubleQuote = '"',
        Hash = '#',
        Percent = '%',
        Dollar = '$',
        Ampersand = '&',
        Quote = '\'',
        LeftParen = '(',
        RightParen = ')',
        Asterisk = '*',
        Plus = '+',
        Comma = ',',
        Minus = '-',
        Period = '.',
        Slash = '/',
        N0 = '0',
        N1 = '1',
        N2 = '2',
        N3 = '3',
        N4 = '4',
        N5 = '5',
        N6 = '6',
        N7 = '7',
        N8 = '8',
        N9 = '9',
        Colon = ':',
        Semicolon = ';',
        Less = '<',
        Equals = '=',
        Greater = '>',
        Question = '?',
        At = '@',

        LeftBracket = '[',
        BackSlash = '\\',
        RightBracket = ']',
        Caret = '^',
        Underscore = '_',
        Backquote = '`',
        A = 'a',
        B = 'b',
        C = 'c',
        D = 'd',
        E = 'e',
        F = 'f',
        G = 'g',
        H = 'h',
        I = 'i',
        J = 'j',
        K = 'k',
        L = 'l',
        M = 'm',
        N = 'n',
        O = 'o',
        P = 'p',
        Q = 'q',
        R = 'r',
        S = 's',
        T = 't',
        U = 'u',
        V = 'v',
        W = 'w',
        X = 'x',
        Y = 'y',
        Z = 'z',

        Capslock = (57 | (1 << 30)),

        F1 = (58 | (1 << 30)),
        F2 = (59 | (1 << 30)),
        F3 = (60 | (1 << 30)),
        F4 = (61 | (1 << 30)),
        F5 = (62 | (1 << 30)),
        F6 = (63 | (1 << 30)),
        F7 = (64 | (1 << 30)),
        F8 = (65 | (1 << 30)),
        F9 = (66 | (1 << 30)),
        F10 = (67 | (1 << 30)),
        F11 = (68 | (1 << 30)),
        F12 = (69 | (1 << 30)),

        PrintScreen = (70 | (1<<30)),
        ScrollLock = (71 | (1<<30)),
        Pause = (72 | (1<<30)),
        Insert = (73 | (1<<30)),
        Home = (74 | (1<<30)),
        PageUp = (75 | (1<<30)),
        Delete = '\x7F',
        End = (77 | (1<<30)),
        PageDown = (78 | (1<<30)),
        Right = (79 | (1<<30)),
        Left = (80 | (1<<30)),
        Down = (81 | (1<<30)),
        Up = (82 | (1<<30)),

        NumlockClear = (83 | (1 << 30)),
        KeypadDivide = (84 | (1 << 30)),
        KeypadMultiply = (85 | (1 << 30)),
        KeypadMinus = (86 | (1 << 30)),
        KeypadPlus = (87 | (1 << 30)),
        KeypadEnter = (88 | (1 << 30)),
        Keypad1 = (89 | (1 << 30)),
        Keypad2 = (90 | (1 << 30)),
        Keypad3 = (91 | (1 << 30)),
        Keypad4 = (92 | (1 << 30)),
        Keypad5 = (93 | (1 << 30)),
        Keypad6 = (94 | (1 << 30)),
        Keypad7 = (95 | (1 << 30)),
        Keypad8 = (96 | (1 << 30)),
        Keypad9 = (97 | (1 << 30)),
        Keypad0 = (98 | (1 << 30)),
        KeypadPeriod = (99 | (1 << 30)),

        LCtrl = (224 | (1 << 30)),
        LShift = (225 | (1 << 30)),
        LAlt = (226 | (1 << 30)),
        LGui = (227 | (1 << 30)),
        RCtrl = (228 | (1 << 30)),
        RShift = (229 | (1 << 30)),
        RAlt = (230 | (1 << 30)),
        RGui = (231 | (1 << 30)),

    };
    ENUM_CLASS(KeyCode);

    ENUM(Flags)
    enum class KeyModifier : u16
    {
        None = 0x0000,
        LShift = 0x0001,
        RShift = 0x0002,
        LCtrl = 0x0040,
        RCtrl = 0x0080,
        LAlt = 0x0100,
        RAlt = 0x0200,
        LGui = 0x0400,
        RGui = 0x0800,
        Num = 0x1000,
        Caps = 0x2000,
        Mode = 0x4000,
        Scroll = 0x8000,

        Ctrl = LCtrl | RCtrl,
        Shift = LShift | RShift,
        Alt = LAlt | RAlt,
        Gui = LGui | RGui,
        LCmd = LGui,
		RCmd = RGui,
    	Cmd = Gui,
    };
    ENUM_CLASS_FLAGS(KeyModifier);

    ENUM()
    enum class MouseButton
    {
        None = 0,
        Left = 1,
        Middle = 2,
        Right = 3,
        Button4 = 4,
        Button5 = 5
    };
    ENUM_CLASS(MouseButton);

    ENUM(Flags)
    enum class MouseButtonMask
    {
	    None = 0,
        Left = BIT((int)MouseButton::Left),
        Middle = BIT((int)MouseButton::Middle),
        Right = BIT((int)MouseButton::Right),
        Button4 = BIT((int)MouseButton::Button4),
        Button5 = BIT((int)MouseButton::Button5)
    };
    ENUM_CLASS_FLAGS(MouseButtonMask);
    
} // namespace CE

#include "CoreInputTypes.rtti.h"