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

        f32 GetDpiScale();

        bool NeedsRepaint();

        void DoPaintIfNeeded();

        bool IsLayerDirty();

    fusioncore_internal:

        void SetLayerDirty(bool value);

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
        FAffineTransform cachedGlobalTransform;

        bool isLayerDirty = false;
        FUIDrawList drawList;
        Array<u32> splitPoints;

        friend class FLayerTree;
        friend class FPainter;
    };

} // namespace CE

#include "FLayer.rtti.h"