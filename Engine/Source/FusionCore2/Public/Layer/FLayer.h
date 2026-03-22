#pragma once

namespace CE
{
    class FLayerTree;

    CLASS()
    class FUSIONCORE_API FLayer : public Object
    {
        CE_CLASS(FLayer, Object)
    public:

        FLayer();

        Ref<FWidget> GetOwningWidget() { return owningWidget.Lock(); }

        Ref<FLayer> GetParentLayer() { return parent.Lock(); }

        f32 GetDpiScale();

        bool NeedsCompositing() { return needsCompositing; }

        bool NeedsRepaint();

        void DoPaintIfNeeded();

        u32 GetChildCount() { return children.GetSize(); }

        Ref<FLayer> GetChild(u32 index) { return children[index]; }

        FUIDrawList* GetDrawList() { return &drawList; }

        u32 GetSplitPointCount() { return splitPoints.GetSize(); }

        u32 GetSplitPoint(u32 index) { return splitPoints[index]; }

        const FAffineTransform& GetTransformInParentSpace() const { return cachedTransformInParentSpace; }

        FAffineTransform GetGlobalTransform();

    protected:

        void DoPaint();

        void DoPaint(FWidget* widget, FPainter& painter);

        FIELD()
        WeakRef<FLayerTree> ownerTree;

        FIELD()
        WeakRef<FLayer> parent;

        FIELD()
        WeakRef<FWidget> owningWidget;

        FIELD()
        Array<Ref<FLayer>> children;

        FIELD()
        FAffineTransform cachedTransformInParentSpace;

        bool needsCompositing = false;
        FUIDrawList drawList;
        Array<u32> splitPoints;

        friend class FLayerTree;
        friend class FPainter;
        friend class FRenderSnapshot;
    };

} // namespace CE

#include "FLayer.rtti.h"