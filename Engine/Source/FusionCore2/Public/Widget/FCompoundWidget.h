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

        void SetChild(Ref<FWidget> child);


		// - Layout -

        Vec2 MeasureContent(Vec2 availableSize) override;

		void ArrangeContent(Vec2 finalSize) override;


    private: // - Internal -

        FIELD()
        Ref<FWidget> m_Child;

    public: // - Fusion Properties - 

        Self& Child(FWidget& widget);

        FUSION_WIDGET;
    };
    
}

#include "FCompoundWidget.rtti.h"
