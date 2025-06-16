#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockTabWell : public FReorderableStack
    {
        CE_CLASS(FDockTabWell, FReorderableStack)
    protected:

        FDockTabWell();

        void Construct() override;

        void OnPaint(FPainter* painter) override;

        void OnItemsRearranged() override;

    public: // - Public API -

        void UpdateTabWell();

        int GetTabIndex(Ref<FDockTabItem> tabItem);

        Ref<FDockTabItem> GetTabItem(int index);

        Ref<FDockspace> GetDockspace() { return owner.Lock(); }

        void SetActiveTab(Ref<FDockTabItem> tabItem);

        void ApplyStyle() override;

    protected: // - Internal -

        WeakRef<FDockspace> owner;

        Array<Ref<FDockTabItem>> tabItems;

    public: // - Fusion Properties -

        FUSION_WIDGET;
        friend class FDockspace;
        friend class FDockTabWellStyle;
        friend class FDockTabItem;
    };
    
}

#include "FDockTabWell.rtti.h"
