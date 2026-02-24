#pragma once

namespace CE
{
    class FWidget;

    CLASS()
    class FUSIONCORE_API FLayer : public Object
    {
        CE_CLASS(FLayer, Object)
    protected:

        FLayer();
        
    public:

		void AddChild(Ref<FLayer> childLayer);

		void RemoveChild(Ref<FLayer> childLayer);

		void SetOwningWidget(Ref<FWidget> widget);

        Ref<FWidget> GetOwningWidget() const { return owningWidget.Lock(); }

		Ref<FLayer> GetParentLayer() const { return parentLayer.Lock(); }

		u32 GetChildCount() const { return (u32)children.GetSize(); }

        Ref<FLayer> GetChildAt(u32 index) const
        {
            if (index >= children.GetSize())
            {
                return nullptr;
            }
            return children[index];
		}

        Ref<FSurface> GetParentSurface();

        void MarkPaintDirty();

        bool NeedsRepaint() const { return needsRepaint; }

    protected:

        FIELD()
        Array<Ref<FLayer>> children;

        FIELD()
		WeakRef<FLayer> parentLayer;

        FIELD()
        WeakRef<FWidget> owningWidget;

        // - Properties -

        bool needsRepaint = true;

    private:

        FIELD()
        Ref<FPainter> painter;
    };
    
} // namespace CE

#include "FLayer.rtti.h"
