#pragma once

namespace CE
{
    ENUM(Flags)
    enum class FWidgetFlags : u8
    {
	    None = 0,
        PaintDirty = BIT(0),
		LayoutDirty = BIT(1),
		Faulted = BIT(2),
		Disabled = BIT(3),
		Hidden = BIT(4),
		ForceOwnLayer = BIT(5)
    };
    ENUM_CLASS_FLAGS(FWidgetFlags)

    struct FWidgetBuilder
    {
        FWidgetBuilder() {}
    };

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

		CE_FORCE_INLINE bool IsPaintDirty() const { return EnumHasAnyFlags(widgetFlags, FWidgetFlags::PaintDirty); }

        CE_FORCE_INLINE bool IsLayoutDirty() const { return EnumHasAnyFlags(widgetFlags, FWidgetFlags::LayoutDirty); }

        CE_FORCE_INLINE bool IsFaulted() const { return EnumHasAnyFlags(widgetFlags, FWidgetFlags::Faulted); }

		CE_FORCE_INLINE bool IsEnabled() const { return !EnumHasAnyFlags(widgetFlags, FWidgetFlags::Disabled); }

		CE_FORCE_INLINE bool IsVisible() const { return !EnumHasAnyFlags(widgetFlags, FWidgetFlags::Hidden); }

    public:

    	// - Layout -

		Vec2 GetLayoutPosition() const { return layoutPosition; }

		Vec2 GetLayoutSize() const { return layoutSize; }

        void SetLayoutPosition(Vec2 newPosition);

		Vec2 GetDesiredSize() const { return desiredSize; }

        virtual bool IsLayoutRoot();

		Vec2 GetMinimumContentSize();

        Vec2 ApplyLayoutConstraints(Vec2 desiredSize);

        virtual Vec2 MeasureContent(Vec2 availableSize);

        virtual void ArrangeContent(Vec2 finalSize);

    	// - Hierarchy -

    	virtual void SetParentSurfaceRecursive(Ref<FSurface> surface);

    	virtual void DetachChild(Ref<FWidget> child) {}

    	void DetachFromParent();

        // - Layer -

        bool IsPaintRoot();

	protected:

        // - Layer -

        bool ShouldOwnLayer();

		void UpdateLayerOwnership();

	public: // - Getters & Setters -

		Ref<FWidget> GetParentWidget() const { return parentWidget.Lock(); }

		Ref<FSurface> GetParentSurface() const { return parentSurface.Lock(); }

    	// For internal use only!
		void SetParentWidget(Ref<FWidget> newParentWidget) { parentWidget = newParentWidget; }

    	// For internal use only!
    	void SetParentSurface(Ref<FSurface> surface) { parentSurface = surface; }
    	
    	// - Callbacks -

		virtual void OnFusionPropertyModified(const Name& propertyName) {}

	private: // - Internal -

		void PromoteToLayerOwner();
		void DemoteFromLayerOwner();
        FLayer* FindNearestAncestorLayer();

        FIELD()
		WeakRef<FWidget> parentWidget;

        FIELD()
		WeakRef<FSurface> parentSurface;

        FIELD()
        Ref<FLayer> ownedLayer;

    protected:

        // - Cache -

        FAffineTransform cachedGlobalTransform;

        // - Layout -

        FIELD()
		Vec2 layoutPosition;

        FIELD()
		Vec2 layoutSize;

        FIELD()
		Vec2 desiredSize;

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

        FUSION_PAINT_PROPERTY(f32, Opacity);

        FUSION_LAYOUT_PROPERTY(CE::HAlign, HAlign);
        FUSION_LAYOUT_PROPERTY(CE::VAlign, VAlign);

        FUSION_PROPERTY_SET(f32, Width)
        {
            return self
                .MinWidth(value)
                .MaxWidth(value);
        }

        FUSION_PROPERTY_SET(f32, Height)
        {
            return self
				.MinHeight(value)
				.MaxHeight(value);
		}

		FUSION_PROPERTY_SET(bool, Enabled)
        {
			bool isCurrentlyEnabled = !EnumHasAnyFlags(self.widgetFlags, FWidgetFlags::Disabled);
			if (value == isCurrentlyEnabled)
                return self;

            if (value)
            {
                self.widgetFlags &= ~FWidgetFlags::Disabled;
            }
            else
            {
                self.widgetFlags |= FWidgetFlags::Disabled;
			}

			self.MarkLayoutDirty();
            return self;
        }

        FUSION_PROPERTY_GET(bool, Enabled)
        {
            return IsEnabled();
        }

        FUSION_PROPERTY_SET(bool, Visible)
        {
			bool isCurrentlyVisible = !EnumHasAnyFlags(self.widgetFlags, FWidgetFlags::Hidden);
			if (value == isCurrentlyVisible)
                return self;

			if (value)
            {
                self.widgetFlags &= ~FWidgetFlags::Hidden;
            }
            else
            {
                self.widgetFlags |= FWidgetFlags::Hidden;
            }
            
            self.MarkPaintDirty();
            return self;
        }

        FUSION_PROPERTY_GET(bool, Visible)
        {
            return IsVisible() || !IsEnabled();
		}

        template<typename TWidget> requires TIsBaseClassOf<FWidget, TWidget>::Value
        TWidget& As()
        {
            TWidget* cast = Object::CastTo<TWidget>(this);
            if (cast == nullptr)
            {
                throw FException(String::Format("FWidget::As(): Cannot cast object ({}) to type {}",
                    GetName(), GetStaticClass<TWidget>()->GetName()),
                    this);
            }
            return (TWidget&)*cast;
        }

        template<typename TWidget> requires TIsBaseClassOf<FWidget, TWidget>::Value
        TWidget& Assign(TWidget*& out)
        {
            out = (TWidget*)this;
            return *out;
        }

        template<typename TWidget> requires TIsBaseClassOf<FWidget, TWidget>::Value
        TWidget& Assign(Ref<TWidget>& out)
        {
            out = (TWidget*)this;
            return *out;
        }

        FUSION_PROPERTY_SET(String, Name)
        {
            self.SetName(value);
            return self;
        }

    private:

        // - Internal -

		FWidgetFlags widgetFlags = FWidgetFlags::None;

        FUSION_WIDGET;
    };

    
} // namespace CE

#include "FWidget.rtti.h"
