#pragma once

namespace CE
{
    ENUM()
    enum class FWrapBoxDirection
    {
        Horizontal,
        Vertical
    };
    ENUM_CLASS(FWrapBoxDirection);

    CLASS()
    class FUSIONCORE_API FWrapBox : public FContainerWidget
    {
        CE_CLASS(FWrapBox, FContainerWidget)
    protected:

        FWrapBox();

        void Construct() override;

    public: // - Public API -

        void CalculateIntrinsicSize() override;

        void PlaceSubWidgets() override;

    protected: // - Internal -


    public: // - Fusion Properties - 

        FUSION_LAYOUT_PROPERTY(FWrapBoxDirection, WrapDirection);
        FUSION_LAYOUT_PROPERTY(CE::HAlign, ContentHAlign);
        FUSION_LAYOUT_PROPERTY(CE::VAlign, ContentVAlign);
        FUSION_LAYOUT_PROPERTY(Vec2, Gap);

        FUSION_WIDGET;
    };
    
}

#include "FWrapBox.rtti.h"
