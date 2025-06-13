#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockTabItem : public FStyledWidget
    {
        CE_CLASS(FDockTabItem, FStyledWidget)
    protected:

        FDockTabItem();

        void Construct() override;

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool IsActive() const { return isActive; }
        bool IsHovered() const { return isHovered; }

    protected: // - Internal -

        Ref<FLabel> tabTitle;

        bool isActive = false;
        bool isHovered = false;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Text, tabTitle, Title);

        FUSION_WIDGET;
    };
    
}

#include "FDockTabItem.rtti.h"
