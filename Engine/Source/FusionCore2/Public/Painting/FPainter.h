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

        // - Path API -

        void PathInsert(Vec2 point);

    private:

        void PathMinMax(Vec2 point);

        using FPathArray = StableDynamicArray<Vec2, 64, false>;

        FLayer* layer = nullptr;
        FUIDrawList* drawList = nullptr;

        FPen currentPen;
        FBrush currentBrush;

        FPathArray path;
        Vec2 pathMin, pathMax;

    };
    
} // namespace CE

