#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockspaceSplitView : public FStyledWidget
    {
        CE_CLASS(FDockspaceSplitView, FStyledWidget)
    protected:

        FDockspaceSplitView();

        void Construct() override;

        void OnPaint(FPainter* painter) override;

        FUNCTION()
        void OnPaintDockingPreview(FPainter* painter);

    public: // - Public API -

        void ApplyStyle() override;

        bool SupportsDropTarget() const override;

        void SetDockingPreviewEnabled(bool enabled, FDockingHintPosition position);

        bool IsDockingPreviewEnabled() { return dockingPreviewEnabled; }

        FDockingHintPosition GetDockingPreviewHintPosition() { return dockingPreviewPosition; }

        Ref<FDockspace> GetDockspace();

        void SetSingleDockWindow(Ref<FDockWindow> dockWindow);

        void RemoveAllContent();

        void SetGuideVisible(bool visible);

        Ref<FHorizontalStack> GetTabWellParent() { return tabWellParent; }

        Ref<FDockTabWell> GetTabWell() { return tabWell; }

		bool IsSingular() const { return childrenSplitViews.IsEmpty(); }

        Ref<FDockWindow> GetTabbedDockWindow(Ref<FDockTabItem> dockTabItem);

        Ref<FDockWindow> GetTabbedDockWindow(int index);
        int GetTabbedDockWindowCount();

        Ref<FDockTabItem> GetDockTabItem(int index);

        void SetActiveTab(Ref<FDockTabItem> tabItem);
        void SetActiveTab(int index);

        bool CanBeDocked(Ref<FDockWindow> dockWindow);
        bool CanBeSplit(Ref<FDockWindow> dockWindow);

        bool CanDetach(Ref<FDockTabItem> dockTabItem);

        void AddDockWindowSplit(FDockingHintPosition splitPosition, Ref<FDockWindow> dockWindow, f32 splitRatio = 0.5f);
        void AddDockWindowSplit(FDockingHintPosition splitPosition, FDockWindow& dockWindow, f32 splitRatio = 0.5f);

        void AddDockWindow(Ref<FDockWindow> dockWindow);

        void AddDockWindow(FDockWindow& dockWindow)
        {
            AddDockWindow(&dockWindow);
        }

        int GetDockedWindowIndex(Ref<FDockWindow> dockedWindow);

        void UpdateTabs();

        Ref<FDockTabItem> DetachItem(Ref<FDockTabItem> dockTabItem);

        bool RemoveDockItemInternal(Ref<FDockTabItem> dockTabItem);

    	bool RemoveDockItem(Ref<FDockTabItem> dockTabItem);

        int GetChildrenSplitCount() const { return childrenSplitViews.GetSize(); }

        Ref<FDockspaceSplitView> GetChildrenSplit(int index) const { return childrenSplitViews[index]; }

		void RemoveChildSplitView(Ref<FDockspaceSplitView> childSplitView);

    protected: // - Internal -

        Array<Ref<FDockspaceSplitView>> childrenSplitViews;

		WeakRef<FDockspaceSplitView> parentSplitView;

        Ref<FSplitBox> splitBox;
        Ref<FDockingGuide> dockingGuide;

        Ref<FHorizontalStack> tabWellParent;
        Ref<FOverlayStack> tabWellOverlay;
        Ref<FDockTabWell> tabWell;

        Ref<FDockTabItem> selectedTab;
        Ref<FStyledWidget> previewWidget;

        Array<Ref<FDockWindow>> tabbedDockWindows;

        WeakRef<FDockspace> ownerDockspace;

        bool dockingPreviewEnabled = false;
        FDockingHintPosition dockingPreviewPosition = FDockingHintPosition::Center;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Margin, tabWell, TabWellMargin);

        FUSION_WIDGET;
        friend class FDockspace;
        friend class FDockTabWell;
        friend class FDockTabItem;
    };
    
}

#include "FDockspaceSplitView.rtti.h"
