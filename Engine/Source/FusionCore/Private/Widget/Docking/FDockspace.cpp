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

        m_DetachedWindowClass = FToolWindow::StaticClass();
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
                FAssignNew(FHorizontalStack, tabWellParent)
                .ContentVAlign(VAlign::Fill)
                (
                    FAssignNew(FOverlayStack, tabWellOverlay)
                    .ContentHAlign(HAlign::Left)
                    .ContentVAlign(VAlign::Fill)
                    .FillRatio(1.0f)
                    (
                        FAssignNew(FDockTabWell, tabWell)
                        .HAlign(HAlign::Fill)
                        .VAlign(VAlign::Fill)
                    )
                ),

                FAssignNew(FDockspaceSplitView, container)
                .FillRatio(1.0f)
            )
        );

        tabWell->owner = this;
    }

    Ref<FDockTabItem> FDockspace::GetDockTabItem(int index)
    {
        if (index < 0 || index >= tabWell->GetTabCount())
            return nullptr;

		return tabWell->GetTabItem(index);
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

    void FDockspace::SetActiveTab(int index)
    {
        if (index < 0 || index >= tabbedDockWindows.GetSize())
            return;

        Ref<FDockTabItem> tabItem = tabWell->GetTabItem(index);
        if (!tabItem)
			return;

        SetActiveTab(tabItem);
    }

    int FDockspace::GetDockedWindowIndex(Ref<FDockWindow> dockedWindow)
    {
		return tabbedDockWindows.IndexOf(dockedWindow);
    }

    void FDockspace::UpdateTabs()
    {
        tabWell->UpdateTabWell();

        container->RemoveAllChildren();

        if (tabbedDockWindows.IsEmpty())
        {
            if (Ref<FFusionContext> context = GetContext())
            {
                context->QueueDestroy();
			}
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
        if (!m_AllowDocking)
            return false;
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

        int neighborIndex = index - 1;
        neighborIndex = Math::Clamp<int>(neighborIndex, 0, tabbedDockWindows.GetSize() - 1);

        Vec2i screenMousePos = InputManager::GetGlobalMousePosition();

        Ref<FWindow> detachedWindow = FusionApplication::Get()->CreateNativeWindow(dockTabItem->Title(), dockTabItem->Title(), 
            512, 512, 
            FWindow::StaticClass(),
        {
            .maximised = false,
            .fullscreen = false,
            .resizable = false,
            .hidden = false,
            .openCentered = false,
            .openPos = screenMousePos,
            .windowFlags = PlatformWindowFlags::DestroyOnClose
        });

        detachedWindow->GetContext()->SetGhosted(true);

        PlatformWindow* nativeWindow = detachedWindow->GetPlatformWindow();
        nativeWindow->SetBorderless(true);
        nativeWindow->SetAlwaysOnTop(true);
        nativeWindow->SetOpacity(0.4f);

        Ref<FDockspace> tempDockspace = nullptr;

        detachedWindow->SetWindowContent(
            FAssignNew(FDockspace, tempDockspace)
            .DockspaceType(DockspaceType())
            .DestroyWhenEmpty(true)
            .AllowDocking(true)
            .AllowSplitting(m_AllowSplitting)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            .FillRatio(1.0f)
        );

        tempDockspace->m_OnCreateDockspace = m_OnCreateDockspace;
        tempDockspace->m_DetachedWindowClass = m_DetachedWindowClass;
        tempDockspace->m_OnWindowSetup = m_OnWindowSetup;

        tempDockspace->originalWindowSize = nativeWindow->GetWindowSize();

        if (Ref<FNativeContext> nativeContext = GetNativeContext())
        {
            tempDockspace->originalWindowSize = nativeContext->GetWindowSize();
        }

        tempDockspace->AddDockWindow(dockWindow);

        Ref<FDockTabItem> newTabItem = tempDockspace->GetDockTabWell()->GetTabItem(0);
        newTabItem->detached = true;

        FusionApplication::Get()->GetRootContext()->SetFocusWidget(newTabItem.Get());

        if (selectedTab == nullptr && neighborIndex < tabWell->GetTabCount() && neighborIndex >= 0)
        {
            SetActiveTab(tabWell->GetTabItem(neighborIndex));
        }

        detachedWindow->SetContextRecursively(detachedWindow->GetContext().Get());

        return newTabItem;
    }

    bool FDockspace::RemoveDockItem(Ref<FDockTabItem> dockTabItem)
    {
        if (!dockTabItem)
            return false;

        int index = tabWell->GetTabIndex(dockTabItem);
        if (index < 0 || index >= tabbedDockWindows.GetSize())
            return false;

        Ref<FDockWindow> dockWindow = tabbedDockWindows[index];
        if (!dockWindow)
            return false;

        if (selectedTab == dockTabItem)
        {
            selectedTab = nullptr;
        }

        tabWell->RemoveTabItem(dockTabItem);
        tabbedDockWindows.RemoveAt(index);

        UpdateTabs();

        return true;
    }

    Ref<FDockWindow> FDockspace::GetTabbedDockWindow(Ref<FDockTabItem> dockTabItem)
    {
        int index = tabWell->GetTabIndex(dockTabItem);

        if (index >= 0 && index < tabbedDockWindows.GetSize())
        {
            return tabbedDockWindows[index];
        }

        return nullptr;
    }


    FDockspace& FDockspace::TabWellOverlayWidget(FWidget& widget)
    {
        tabWellOverlay->AddChild(widget);

        return *this;
    }

}

