#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockTabItem : public FReorderableStackItem
    {
        CE_CLASS(FDockTabItem, FReorderableStackItem)
    protected:

        FDockTabItem();

        void Construct() override;

        void HandleEvent(FEvent* event) override;

    public: // - Public API -

        bool CanBeDetached() override;

        bool DetachItem() override;

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDragEvents() const override;

        bool IsActive() const { return isActive; }
        bool IsHovered() const { return isHovered; }

        void SetActiveTab();

        void ApplyStyle() override;

    protected: // - Internal -

        Ref<FLabel> tabTitle;
        WeakRef<FDockTabWell> owner;
        f32 startMousePosX = 0;

        bool isActive = false;
        bool isHovered = false;
        bool shouldDetach = false;
        bool isOutside = false;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Text, tabTitle, Title);

        FUSION_WIDGET;
        friend class FDockTabWell;
    };
    
}

#include "FDockTabItem.rtti.h"
