#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FCompoundWidget : public FWidget
    {
        CE_CLASS(FCompoundWidget, FWidget)
    protected:

        FCompoundWidget();

    public: 
    	
    	// - Public API -

        void SetParentSurfaceRecursive(Ref<FSurface> surface) override;

        void DetachChild(Ref<FWidget> child) override;

        Ref<FWidget> GetChild() const { return m_Child; }

        u32 GetChildCount() override { return m_Child.IsValid() ? 1 : 0; }

        Ref<FWidget> GetChildAt(u32 index) override { return index == 0 ? m_Child : nullptr; }

        void SetChild(Ref<FWidget> child);

		// - Layout -

        Vec2 MeasureContent(Vec2 availableSize) override;

		void ArrangeContent(Vec2 finalSize) override;

        // - Paint -


    private: // - Internal -

        FIELD()
        Ref<FWidget> m_Child;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_SET(FWidget&, Child)
        {
            SetChild(&value);
            return *this;
        }

        FUSION_PAINT_PROPERTY(bool, ClipContent);

        FUSION_WIDGET;
    };
    
}

#include "FCompoundWidget.rtti.h"
