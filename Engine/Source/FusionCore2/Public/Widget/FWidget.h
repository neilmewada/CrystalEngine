#pragma once

namespace CE
{
    ENUM(Flags)
    enum class FWidgetFlags : u8
    {
	    None = 0,
        PaintDirty = BIT(0),
		LayoutDirty = BIT(1),
		Faulted = BIT(2)
    };
    ENUM_CLASS_FLAGS(FWidgetFlags)

    CLASS()
    class FUSIONCORE_API FWidget : public Object
    {
        CE_CLASS(FWidget, CE::Object)
    protected:

        FWidget();

    	void OnAfterConstruct() override final;

		virtual void Construct();

    public: // - Flags -

        virtual void MarkPaintDirty();

        virtual void MarkLayoutDirty();

		CE_FORCE_INLINE bool IsPaintDirty() const { return EnumHasAnyFlags(flags, FWidgetFlags::PaintDirty); }

        CE_FORCE_INLINE bool IsLayoutDirty() const { return EnumHasAnyFlags(flags, FWidgetFlags::LayoutDirty); }

        CE_FORCE_INLINE bool IsFaulted() const { return EnumHasAnyFlags(flags, FWidgetFlags::Faulted); }

    public:

    	// - Layout -

		Vec2 GetLayoutPosition() const { return layoutPosition; }

		Vec2 GetLayoutSize() const { return layoutSize; }

		void SetLayoutPosition(Vec2 newPosition) { layoutPosition = newPosition; }

        virtual bool IsLayoutRoot();

    	virtual bool IsPaintRoot();

		Vec2 GetMinimumContentSize();

        Vec2 ApplyLayoutConstraints(Vec2 desiredSize);

        virtual Vec2 MeasureContent(Vec2 availableSize);

        virtual void ArrangeContent(Vec2 finalSize);

    	// - Hierarchy -

    	virtual void SetParentSurfaceRecursive(Ref<FSurface> surface);

    	virtual void DetachChild(Ref<FWidget> child) {}

    	void DetachFromParent();

	public: // - Getters & Setters -

		Ref<FWidget> GetParentWidget() const { return parentWidget.Lock(); }

		Ref<FSurface> GetParentSurface() const { return parentSurface.Lock(); }

    	// For internal use only!
		void SetParentWidget(Ref<FWidget> newParentWidget) { parentWidget = newParentWidget; }

    	// For internal use only!
    	void SetParentSurface(Ref<FSurface> surface) { parentSurface = surface; }

	protected: // - Callbacks -

		virtual void OnFusionPropertyModified(const Name& propertyName) {}


	private: // - Internal -

        FIELD()
		WeakRef<FWidget> parentWidget;

        FIELD()
		WeakRef<FSurface> parentSurface;

    protected:

        // - Cache -

        FAffineTransform cachedGlobalTransform;

        // - Layout -

        FIELD()
		Vec2 layoutPosition;

        FIELD()
		Vec2 layoutSize;


    public: // - Fusion Properties -

		// Does not affect layout. Used for freeform transformations like rotation or translation that should not cause a layout pass when modified.
        FUSION_PAINT_PROPERTY(FAffineTransform, Transform);

        FUSION_LAYOUT_PROPERTY(FMargin, Margin);
        FUSION_LAYOUT_PROPERTY(FMargin, Padding);

        FUSION_LAYOUT_PROPERTY(f32, MinWidth);
        FUSION_LAYOUT_PROPERTY(f32, MinHeight);

        FUSION_LAYOUT_PROPERTY(f32, MaxWidth);
        FUSION_LAYOUT_PROPERTY(f32, MaxHeight);

        FUSION_LAYOUT_PROPERTY(f32, FillRatio);

        FUSION_LAYOUT_PROPERTY(CE::HAlign, HAlign);
        FUSION_LAYOUT_PROPERTY(CE::VAlign, VAlign);

        Self& Width(f32 width);
        Self& Height(f32 height);

    private:

        // - Internal -

		FWidgetFlags flags = FWidgetFlags::None;

        FUSION_WIDGET;
    };

} // namespace CE

#include "FWidget.rtti.h"
