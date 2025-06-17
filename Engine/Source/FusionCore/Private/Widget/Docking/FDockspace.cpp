#include "FusionCore.h"

namespace CE
{
    FDockspaceFilter::Self& FDockspaceFilter::WithDockTypeMask(FDockTypeMask mask)
    {
        this->allowedDockTypes = mask;
        return *this;
    }

    FDockspaceFilter::Self& FDockspaceFilter::WithAllowedDockspaces(const Array<FDockId>& dockspaceIds)
    {
        this->allowedDockspaces = dockspaceIds;
        return *this;
    }

    FDockspace::FDockspace()
    {
        m_AllowDocking = true;
        m_AllowSplitting = false;
        m_DockspaceType = FDockTypeMask::Minor;
        m_DestroyWhenEmpty = false;

        detachedWindowClass = FWindow::StaticClass();
    }

    void FDockspace::Construct()
    {
        Super::Construct();

        dockId = FDockId::New();

        Child(
            FNew(FVerticalStack)
            .ContentHAlign(HAlign::Fill)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
                FAssignNew(FDockTabWell, tabWell),

                FAssignNew(FDockspaceSplitView, container)
                .FillRatio(1.0f)
            )
        );

        tabWell->owner = this;
    }

    void FDockspace::SetActiveTab(Ref<FDockTabItem> tabItem)
    {
        if (tabWell->GetTabIndex(tabItem) == -1)
        {
            tabItem = tabWell->GetTabItem(0);
        }

        selectedTab = tabItem;

        UpdateTabs();

        ApplyStyle();
    }

    void FDockspace::UpdateTabs()
    {
        tabWell->UpdateTabWell();

        container->RemoveAllChildren();

        if (tabbedDockWindows.IsEmpty())
        {
            GetContext()->QueueDestroy();
	        return;
        }

        int activeTabIndex = tabWell->GetTabIndex(selectedTab);
        activeTabIndex = Math::Clamp<int>(activeTabIndex, 0, tabbedDockWindows.GetSize() - 1);

        container->AddChild(tabbedDockWindows[activeTabIndex].Get());
        tabbedDockWindows[activeTabIndex]->FillRatio(1.0f);
    }

    void FDockspace::AddDockWindow(Ref<FDockWindow> dockWindow)
    {
        if (tabbedDockWindows.Exists(dockWindow))
            return;

        tabbedDockWindows.Add(dockWindow);
        tabWell->UpdateTabWell();

        if (tabbedDockWindows.GetSize() == 1)
        {
            SetActiveTab(tabWell->GetTabItem(0));
        }
    }

    bool FDockspace::CanDetach(Ref<FDockTabItem> dockTabItem)
    {
        if (!m_DestroyWhenEmpty && tabbedDockWindows.GetSize() <= 1)
            return false;

        int index = tabWell->GetTabIndex(dockTabItem);
        if (index < 0)
            return false;

        return true;
    }

    Ref<FDockTabItem> FDockspace::DetachItem(Ref<FDockTabItem> dockTabItem)
    {
        if (!dockTabItem)
            return nullptr;

        int index = tabWell->GetTabIndex(dockTabItem);
        if (index < 0 || index >= tabbedDockWindows.GetSize())
            return nullptr;

        Ref<FDockWindow> dockWindow = tabbedDockWindows[index];
        if (!dockWindow)
            return nullptr;

        if (selectedTab == dockTabItem)
        {
            selectedTab = nullptr;
		}

		tabWell->RemoveTabItem(dockTabItem);
		tabbedDockWindows.RemoveAt(index);

        UpdateTabs();

        Ref<FWindow> detachedWindow = FusionApplication::Get()->CreateNativeWindow(dockTabItem->Title(), dockTabItem->Title(), 
            512, 512, 
            detachedWindowClass,
        {
            .maximised = false,
            .fullscreen = false,
            .resizable = false,
            .hidden = false,
            .windowFlags = PlatformWindowFlags::DestroyOnClose
        });

        PlatformWindow* nativeWindow = detachedWindow->GetPlatformWindow();
        nativeWindow->SetBorderless(true);
        nativeWindow->SetAlwaysOnTop(true);
        nativeWindow->SetOpacity(0.4f);

        Ref<FDockspace> newDockspace = nullptr;

        detachedWindow->SetWindowContent(
            FAssignNew(FDockspace, newDockspace)
            .DockspaceType(DockspaceType())
            .DestroyWhenEmpty(true)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            .FillRatio(1.0f)
        );

        newDockspace->AddDockWindow(dockWindow);

        Ref<FDockTabItem> newTabItem = newDockspace->GetDockTabWell()->GetTabItem(0);
        newTabItem->detached = true;

        FusionApplication::Get()->GetRootContext()->SetFocusWidget(newTabItem.Get());

        return newTabItem;
    }

}

