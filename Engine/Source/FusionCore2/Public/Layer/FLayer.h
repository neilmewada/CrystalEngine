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

        bool NeedsRepaint();

        void DoPaintIfNeeded();

        bool IsLayerDirty();

        u32 GetChildCount() { return children.GetSize(); }

        Ref<FLayer> GetChild(u32 index) { return children[index]; }

        FUIDrawList* GetDrawList() { return &drawList; }

        u32 GetSplitPointCount() { return splitPoints.GetSize(); }

        u32 GetSplitPoint(u32 index) { return splitPoints[index]; }

        RHI::ShaderResourceGroup* GetLayerSrg() const { return layerSrg; }

        void UpdateLayerSrg(u32 frameIndex);

    fusioncore_internal:

        void SetLayerDirty(bool value);

    protected:

        void OnAfterConstruct() override;

        void OnBeforeDestroy() override;

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

        RHI::ShaderResourceGroup* layerSrg = nullptr;
        StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> buffersPerImage;

        friend class FLayerTree;
        friend class FPainter;
    };

} // namespace CE

#include "FLayer.rtti.h"