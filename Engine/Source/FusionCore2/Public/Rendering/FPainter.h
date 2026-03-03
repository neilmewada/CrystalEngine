#pragma once

namespace CE
{
    class FLayer;

    class FUSIONCORE_API FPainter
    {
        CE_NO_COPY_MOVE(FPainter);
    private:

        FPainter(FLayer* layer);

    public:

        // - Public API 0

        void Begin();

        void End();

        void PushCoordinateSpace(const FAffineTransform& transform);
        void PopCoordinateSpace();

    private:

        FLayer* owningLayer = nullptr;
        FUIDrawList* drawList = nullptr;
        bool isRecording = false;

        FPen currentPen;
        FBrush currentBrush;

        struct ClipRegion
        {
            Rect aabbRect;
            Matrix4x4 clipInverseTransform;
            Vec2 clipHalfSize;

            bool IsAabb() const
            {
                return !aabbRect.IsEmpty();
            }
        };

        struct CoordinateSpace
        {
            FAffineTransform transform;
        };

        using FCoordinateSpaceStack = StableDynamicArray<CoordinateSpace, 128, false>;

        FCoordinateSpaceStack coordinateSpaceStack;

        Array<ClipRegion> clipStack;

        FUIBlendMode currentBlendMode = FUIBlendMode::Normal;

        friend class FLayer;
    };
    
} // namespace CE

