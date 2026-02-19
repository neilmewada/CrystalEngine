#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FWidget : public Object
    {
        CE_CLASS(FWidget, CE::Object)
    protected:

        FWidget();
        
    public:

        // - Flags -

        virtual void MarkPaintDirty();

        virtual void MarkLayoutDirty();

        // - Layout -

		Vec2 GetLayoutPosition() const { return layoutPosition; }

		Vec2 GetLayoutSize() const { return layoutSize; }

		virtual Vec2 MeasureContent(Vec2 availableSize) { return Vec2(0, 0); }

		virtual void ArrangeContent(Vec2 finalSize) {}

    protected:

		virtual void OnFusionPropertyModified(const Name& propertyName) {}

    private:

        FIELD()
		WeakRef<FWidget> parentWidget;

        FIELD()
		WeakRef<FSurface> parentSurface;

    protected:

        // - Layout -

        FIELD()
		Vec2 layoutPosition;

        FIELD()
		Vec2 layoutSize;

        // - Cache -

		FAffineTransform cachedGlobalTransform;

    public:

		// - Fusion Properties -

        FUSION_PROPERTY(FAffineTransform, Transform);

        FUSION_LAYOUT_PROPERTY(f32, MinWidth);
        FUSION_LAYOUT_PROPERTY(f32, MinHeight);

        FUSION_LAYOUT_PROPERTY(f32, MaxWidth);
        FUSION_LAYOUT_PROPERTY(f32, MaxHeight);

        FUSION_WIDGET;
    };
    
} // namespace CE

#include "FWidget.rtti.h"
