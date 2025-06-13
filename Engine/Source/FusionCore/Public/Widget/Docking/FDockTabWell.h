#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockTabWell : public FStyledWidget
    {
        CE_CLASS(FDockTabWell, FStyledWidget)
    protected:

        FDockTabWell();

        void Construct() override;

    public: // - Public API -

        void UpdateTabWell();

        int GetTabIndex(Ref<FDockTabItem> tabItem);

        Ref<FDockTabItem> GetTabItem(int index);

        Ref<FDockspace> GetDockspace() { return owner.Lock(); }

        void SetActiveTab(Ref<FDockTabItem> tabItem);

        void ApplyStyle() override;

    protected: // - Internal -

        WeakRef<FDockspace> owner;

        Ref<FHorizontalStack> container;

        Array<Ref<FDockTabItem>> tabItems;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FDockspace;
        friend class FDockTabWellStyle;
    };
    
}

#include "FDockTabWell.rtti.h"
