#pragma once

namespace CE
{
    using FUIVertexArray        = StableDynamicArray<FUIVertex,         1024, false>;
    using FUIIndexArray         = StableDynamicArray<FUIIndex,          1024, false>;
    using FUIDrawItemArray      = StableDynamicArray<FUIDrawItem,       512,  false>;
    using FUIClipRectArray	    = StableDynamicArray<FUIClipRect,       64,   false>;
    using FUIGradientStopArray  = StableDynamicArray<FUIGradientStop,   64,   false>;
    using FUIDrawCmdArray	    = StableDynamicArray<FUIDrawCmd,        64,   false>;

    class FUSIONCORE_API FUIDrawList final : public IntrusiveBase
    {
    public:

        // -  Public API -

        void Clear();

        void Finalize();

        // Low-level write — called by FPainter's tessellation
        void PrimWriteVtx(Vec2 pos, Vec2 uv, u32 color, u32 drawItemIndex);
        void PrimWriteIdx(FUIIndex idx);


        // Current vertex count — used to compute relative index offsets
        u32 GetCurrentVertexCount() const { return vertexArray.GetCount(); }

        FUIDrawCmd& AcquireDrawCmd(FUIBlendMode blendMode, Rect scissorRect, u32 customShaderId = 0);

        u32 AddDrawItem(const FUIDrawItem& item);

        // - Data -

        FUIVertexArray vertexArray;
        FUIIndexArray indexArray;
        FUIDrawItemArray drawItemArray;
        FUIClipRectArray clipRectArray;
        FUIGradientStopArray gradientStopArray;
        FUIDrawCmdArray drawCmdArray;

    };
    
}
