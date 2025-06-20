#pragma once

namespace CE
{
    class FDockTabItem;
    class FDockspace;
    class FDockspaceSplitView;

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

		bool SupportsDropTarget() const override { return true; }

        void UpdateTabWell();

        int GetTabIndex(Ref<FDockTabItem> tabItem);

		int GetTabCount() const { return tabItems.GetSize(); }

        Ref<FDockTabItem> GetTabItem(int index);

		void RemoveTabItem(Ref<FDockTabItem> tabItem);

        Ref<FDockspace> GetDockspace();

        Ref<FDockspaceSplitView> GetDockspaceSplitView();

        void SetActiveTab(Ref<FDockTabItem> tabItem);

        void ApplyStyle() override;

    protected: // - Internal -

        WeakRef<FDockspaceSplitView> owner;
        WeakRef<FDockspace> ownerDockspace;

        Array<Ref<FDockTabItem>> tabItems;

    public: // - Fusion Properties -

        FUSION_WIDGET;
        friend class FDockspace;
        friend class FDockTabWellStyle;
        friend class FDockTabItem;
        friend class FDockspaceSplitView;
    };
    
}

#include "FDockTabWell.rtti.h"
