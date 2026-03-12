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

        f32 GetCurrentOpacity() const { return opacityStack.IsEmpty() ? 1.0f : opacityStack.Last(); }

        void PushTransform(const FAffineTransform& transform);
        void PopTransform();

        FAffineTransform GetCurrentTransform();

        void SetPen(const FPen& pen) { currentPen = pen; }

        void SetBrush(const FBrush& brush) { currentBrush = brush; }

        // - Path API -

        void PathClear();

        void PathInsert(Vec2 point);
        void PathArcTo(const Vec2& center, float radius, float startAngleRadians, float endAngleRadians);
        void PathArcToFast(const Vec2& center, float radius, float startAngleRadians, float endAngleRadians);

        void PathBezierCubicCurveTo(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, int numSegments = 0);
        void PathQuadraticCubicCurveTo(const Vec2& p1, const Vec2& p2, const Vec2& p3, int numSegments = 0);

        bool PathFill(bool antiAliased);
        bool PathStroke(bool closed, bool antiAliased);
        bool PathFillAndStroke(bool antiAliased);

    private:

        int CalculateNumCircleSegments(float radius) const;

        // - Path Internals -

        void PathMinMax(Vec2 point);
        void PathArcTo(const Vec2& center, float radius, float startAngleRadians, float endAngleRadians, int numSegments);
        void PathArcToFastInternal(const Vec2& center, float radius, int sampleMin, int sampleMax, int step);

        bool PathStrokeInternal(bool closed, bool antiAliased);
        bool PathFillInternal(bool antiAliased);

        // - Drawing Internals -

        using FPathArray = StableDynamicArray<Vec2, 128, false>;
        using FOpacityStack = StableDynamicArray<f32, 32, false>;
        using FTransformStack = StableDynamicArray<FAffineTransform, 128, false>;

        static constexpr u32 ArcFastTableSize = 48;

        FLayer* layer = nullptr;
        FUIDrawList* drawList = nullptr;

        FPen currentPen;
        FBrush currentBrush;

        FPathArray path;
        Vec2 pathMin, pathMax;

        FOpacityStack opacityStack;
        FTransformStack transformStack;

        f32 circleSegmentMaxError = 0.2f;
        f32 curveTessellationTolerance = 1.25f;

        Vec2 arcFastVertex[ArcFastTableSize] = {};
        float arcFastRadiusCutoff = 0;
    };
    
} // namespace CE

