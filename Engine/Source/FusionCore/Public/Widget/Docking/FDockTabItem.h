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

        void HandleEvent(FEvent* event) override;

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDragEvents() const override;

        bool IsActive() const { return isActive; }
        bool IsHovered() const { return isHovered; }

        void SetActiveTab();

        void ApplyStyle() override;

    protected: // - Internal -

        Ref<FLabel> tabTitle;
        WeakRef<FDockTabWell> owner;

        bool isActive = false;
        bool isHovered = false;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Text, tabTitle, Title);

        FUSION_WIDGET;
        friend class FDockTabWell;
    };
    
}

#include "FDockTabItem.rtti.h"
