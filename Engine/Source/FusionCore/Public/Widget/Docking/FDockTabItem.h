#pragma once

namespace CE
{
    class FDockWindow;

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

        Ref<FReorderableStackItem> DetachItem() override;

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDragEvents() const override;

        bool IsActive() const { return isActive; }
        bool IsHovered() const { return isHovered; }
        bool IsDetached() const { return detached; }
        bool IsJoined() const { return joined; }

        void SetActiveTab();

        void ApplyStyle() override;

        bool CloseWindow();

		Ref<FDockWindow> GetDockWindow() const { return dockWindow.Lock(); }

    protected: // - Internal -

        Ref<FImage> iconImage;
        Ref<FLabel> tabTitle;
        Ref<FImageButton> closeButton;

        WeakRef<FDockTabWell> owner;
        f32 startMousePosX = 0;

        Ref<FHorizontalStack> container;

        WeakRef<FDockspaceSplitView> guideDockspaceSplitView;

		WeakRef<FDockWindow> dockWindow;

        bool isActive = false;
        bool isHovered = false;
        bool detached = false;
        bool canBeDetached = true;

    	bool joined = false;
        f32 joinPosOffset = 0;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Text, tabTitle, Title);

        FUSION_PROPERTY_WRAPPER2(Enabled, closeButton, CloseButtonEnabled);
        FUSION_PROPERTY_WRAPPER2(Visible, closeButton, CloseButtonVisible);

        FUSION_WIDGET;
        friend class FDockTabWell;
        friend class FDockspaceSplitView;
        friend class FDockspace;
    };
    
}

#include "FDockTabItem.rtti.h"
