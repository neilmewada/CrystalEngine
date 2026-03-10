#pragma once

namespace CE
{
    class FWidget;
    class FLayer;

    class FUSIONCORE_API FPainter
    {
        CE_NO_COPY_MOVE(FPainter);
    public:

        FPainter(FLayer* layer);

        // - Public API -

        FLayer* GetLayer() const { return layer; }

        void PushTransform(const FAffineTransform& transform);
        void PopTransform();

        FAffineTransform GetCurrentTransform();

        void SetPen(const FPen& pen) { currentPen = pen; }

        void SetBrush(const FBrush& brush) { currentBrush = brush; }

    private:

        Array<FAffineTransform> transformStack;

        FLayer* layer = nullptr;

        FPen currentPen;
        FBrush currentBrush;

    };
    
} // namespace CE

