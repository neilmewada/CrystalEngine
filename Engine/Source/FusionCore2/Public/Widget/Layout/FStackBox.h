#pragma once

namespace CE
{
    ENUM()
    enum class FStackDirection
    {
		Horizontal,
		Vertical
    };
    ENUM_CLASS(FStackDirection);

    CLASS()
    class FUSIONCORE_API FStackBox : public FContainerWidget
    {
        CE_CLASS(FStackBox, FContainerWidget)
    protected:

        FStackBox();

    public: 
    	
    	// - Public API -

        void SetParentSurfaceRecursive(Ref<FSurface> surface) override;

		// - Layout -

		Vec2 MeasureContent(Vec2 availableSize) override;

        void ArrangeContent(Vec2 finalSize) override;

    protected: // - Internal -

    public: // - Fusion Properties - 

        FUSION_LAYOUT_PROPERTY(FStackDirection, StackDirection);
		FUSION_LAYOUT_PROPERTY(f32, Spacing);
        FUSION_LAYOUT_PROPERTY(CE::HAlign, ContentHAlign);
        FUSION_LAYOUT_PROPERTY(CE::VAlign, ContentVAlign);

        FUSION_WIDGET;
    };

    CLASS()
    class FUSIONCORE_API FVerticalStack : public FStackBox
    {
        CE_CLASS(FVerticalStack, FStackBox)
    public:

        FVerticalStack()
        {
			m_StackDirection = FStackDirection::Vertical;
        }

    protected:

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

	};

    CLASS()
    class FUSIONCORE_API FHorizontalStack : public FStackBox
    {
        CE_CLASS(FHorizontalStack, FStackBox)
    public:

        FHorizontalStack()
        {
			m_StackDirection = FStackDirection::Horizontal;
        }

    protected:

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

    };
    
}

#include "FStackBox.rtti.h"
