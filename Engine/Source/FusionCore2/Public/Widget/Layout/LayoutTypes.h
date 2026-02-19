#pragma once

namespace CE
{

    ENUM()
    enum class HAlign : u8
    {
        Auto = 0,
        Fill,
        Left,
        Center,
        Right
    };

    ENUM()
    enum class VAlign : u8
    {
        Auto = 0,
        Fill,
        Top,
        Center,
        Bottom
    };
    
} // namespace CE

#include "LayoutTypes.rtti.h"