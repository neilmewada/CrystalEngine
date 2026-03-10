#pragma once

namespace CE
{
    using FUIVertexArray        = StableDynamicArray<FUIVertex,         1024, false>;
    using FUIIndexArray         = StableDynamicArray<FUIIndex,          1024, false>;
    using FUIDrawItemArray      = StableDynamicArray<FUIDrawItem,       512,  false>;
    using FUIClipRectArray	    = StableDynamicArray<FUIClipRect,       64,   false>;
    using FUIGradientStopArray  = StableDynamicArray<FUIGradientStop,   64,   false>;
    using FUIDrawCmdArray	    = StableDynamicArray<FUIDrawCmd,        64,   false>;
    using FTransformStack       = StableDynamicArray<FAffineTransform,  128,  false>;

    class FUSIONCORE_API FUIDrawList final : public IntrusiveBase
    {
    public:

        // -  Public API -

        void Clear();
        void Finalize();

        SIZE_T GetCurrentDrawCmdCount() const { return drawCmdArray.GetCount(); }

        // Current vertex count — used to compute relative index offsets
        u32 GetCurrentVertexCount() const { return vertexArray.GetCount(); }

        FUIDrawCmd& NewDrawCmd();

        FUIDrawCmd& AcquireDrawCmd(FUIBlendMode blendMode, Rect scissorRect, u32 customShaderId = 0);

        u32 AddDrawItem(const FUIDrawItem& item);

        void PushTransform(const FAffineTransform& transform);
        void PopTransform();

        FAffineTransform GetCurrentTransform();

    private:

        void PrimReserve(int vertexCount, int indexCount);

        void PrimRect(const Rect& quad, u32 color, Vec2* uvs, u32 drawItemIndex);

        // - Data -

        FUIVertexArray vertexArray;
        FUIIndexArray indexArray;
        FUIDrawItemArray drawItemArray;
        FUIClipRectArray clipRectArray;
        FUIGradientStopArray gradientStopArray;
        FUIDrawCmdArray drawCmdArray;

        FTransformStack transformStack;

        FUIVertex* vertexWritePtr = nullptr;
        FUIIndex* indexWritePtr = nullptr;

        // Start offset of current vertex
        FUIIndex vertexCurrentIdx = 0;

        friend class FPainter;
    };
    
}
