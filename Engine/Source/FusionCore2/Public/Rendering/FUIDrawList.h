#pragma once

namespace CE
{
    using FUIVertexArray        = StableDynamicArray<FUIVertex,         128, false>;
    using FUIIndexArray         = StableDynamicArray<FUIIndex,          128, false>;
    using FUIDrawItemArray      = StableDynamicArray<FUIDrawItem,       128, false>;
    using FUIClipRectArray	    = StableDynamicArray<FUIClipRect,       64,  false>;
    using FUIGradientStopArray  = StableDynamicArray<FUIGradientStop,   64,  false>;
    using FUIDrawCmdArray	    = StableDynamicArray<FUIDrawCmd,        32,  false>;

    class FUSIONCORE_API FUIDrawList final : public IntrusiveBase
    {
    public:


        FUIVertexArray vertexArray;
        FUIIndexArray indexArray;
        FUIDrawItemArray drawItemArray;
        FUIClipRectArray clipRectArray;
        FUIGradientStopArray gradientStopArray;
        FUIDrawCmdArray drawCmdArray;

    };
    
}
