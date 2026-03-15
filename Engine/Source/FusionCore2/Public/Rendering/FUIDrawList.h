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

        SIZE_T GetCurrentDrawCmdCount() const { return drawCmdArray.GetCount(); }

        // Current vertex count — used to compute relative index offsets
        u32 GetCurrentVertexCount() const { return vertexArray.GetCount(); }

        FUIDrawCmd& NewDrawCmd();

        FUIDrawCmd& AcquireDrawCmd();

        u32 AddDrawItem(const FUIDrawItem& item);

        void AddPolyLine(const Vec2* points, int numPoints, u32 color, f32 thickness, bool closed, bool antiAliased, u32 drawItemIndex = 0);

        void AddConvexPolyFilled(const Vec2* points, int numPoints, u32 color, bool antiAliased, Rect* minMaxPos, u32 drawItemIndex = 0);

        void PrimReserve(int vertexCount, int indexCount);

        // - Types & Constants -

        using FTempPointsArray = StableDynamicArray<Vec2, 128, false>;
        static constexpr u32 ColorAlphaMask = 0xff000000;

        // - Data -

        FUIVertexArray vertexArray;
        FUIIndexArray indexArray;
        FUIDrawItemArray drawItemArray;
        FUIClipRectArray clipRectArray;
        FUIGradientStopArray gradientStopArray;
        FUIDrawCmdArray drawCmdArray;

        FTempPointsArray temporaryPoints;
        FUIVertex* vertexWritePtr = nullptr;
        FUIIndex* indexWritePtr = nullptr;

        // Start offset of current vertex
        FUIIndex vertexCurrentIdx = 0;

        float fringeScale = 1.0f;

        // Draw Cmd
        FUIBlendMode blendMode = FUIBlendMode::Normal;

        friend class FPainter;
    };
    
}
