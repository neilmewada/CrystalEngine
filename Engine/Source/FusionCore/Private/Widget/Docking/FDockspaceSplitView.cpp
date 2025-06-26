#include "FusionCore.h"

namespace CE
{

    FDockspaceSplitView::FDockspaceSplitView()
    {

    }

    void FDockspaceSplitView::Construct()
    {
        Super::Construct();

        Child(
            FNew(FOverlayStack)
            .ContentHAlign(HAlign::Fill)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
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

                    FAssignNew(FSplitBox, splitBox)
                    .SplitterBackground(Color::Clear())
                    .SplitterSize(7.0f)
                    .SplitterDrawRatio(1.0f)
                    .HAlign(HAlign::Fill)
                    .FillRatio(1.0f)
                ),

                FAssignNew(FStyledWidget, previewWidget)
                .OnPaintContentOverlay(FUNCTION_BINDING(this, OnPaintDockingPreview))
                .IgnoreHitTest(true)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill),

                FAssignNew(FDockingGuide, dockingGuide)
                .HAlign(HAlign::Center)
                .VAlign(VAlign::Center)
                .Enabled(false)
            )
        );

        tabWell->owner = this;
        dockingGuide->ownerDockspaceSplitView = this;
    }

    void FDockspaceSplitView::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);
    }

    Ref<FDockWindow> FDockspaceSplitView::GetTabbedDockWindow(Ref<FDockTabItem> dockTabItem)
    {
        int index = tabWell->GetTabIndex(dockTabItem);

        if (index >= 0 && index < tabbedDockWindows.GetSize())
        {
            return tabbedDockWindows[index];
        }

        return nullptr;
    }

    Ref<FDockWindow> FDockspaceSplitView::GetTabbedDockWindow(int index)
    {
        if (index < 0 || index >= tabbedDockWindows.GetSize())
            return nullptr;

        return tabbedDockWindows[index];
    }

    int FDockspaceSplitView::GetTabbedDockWindowCount()
    {
        return tabbedDockWindows.GetSize();
    }

    Ref<FDockTabItem> FDockspaceSplitView::GetDockTabItem(int index)
    {
        if (index < 0 || index >= tabWell->GetTabCount())
            return nullptr;

        return tabWell->GetTabItem(index);
    }

    void FDockspaceSplitView::SetActiveTab(Ref<FDockTabItem> tabItem)
    {
        if (tabWell->GetTabIndex(tabItem) == -1)
        {
            tabItem = tabWell->GetTabItem(0);
        }

        selectedTab = tabItem;

        UpdateTabs();

        ApplyStyle();
    }

    void FDockspaceSplitView::SetActiveTab(int index)
    {
        if (index < 0 || index >= tabbedDockWindows.GetSize())
            return;

        Ref<FDockTabItem> tabItem = tabWell->GetTabItem(index);
        if (!tabItem)
            return;

        SetActiveTab(tabItem);
    }

    bool FDockspaceSplitView::CanBeDocked(Ref<FDockWindow> dockWindow)
    {
        if (Ref<FDockspace> dockspace = GetDockspace())
        {
            return dockspace->CanBeDocked(dockWindow);
        }

        return false;
    }

    bool FDockspaceSplitView::CanBeSplit(Ref<FDockWindow> dockWindow)
    {
        if (!IsSingular())
            return false;

        if (Ref<FDockspace> dockspace = GetDockspace())
        {
            return dockspace->CanBeDocked(dockWindow);
        }

        return false;
    }

    bool FDockspaceSplitView::CanDetach(Ref<FDockTabItem> dockTabItem)
    {
        if (Ref<FDockspace> dockspace = GetDockspace())
        {
            if (!dockspace->AllowDocking())
                return false;
            if (parentSplitView.IsNull() && !dockspace->DestroyWhenEmpty() && tabbedDockWindows.GetSize() <= 1)
                return false;

            int index = tabWell->GetTabIndex(dockTabItem);
            if (index < 0)
                return false;

            return true;
		}

        return false;
    }

    void FDockspaceSplitView::RemoveChildSplitView(Ref<FDockspaceSplitView> childSplitView)
    {
        int index = childrenSplitViews.IndexOf(childSplitView);
		if (index < 0 || index >= childrenSplitViews.GetSize())
            return;

        splitBox->RemoveChild(childSplitView.Get());

        if (splitBox->GetChildCount() == 1)
        {
            if (Ref<FDockspaceSplitView> lastSplitView = CastTo<FDockspaceSplitView>(splitBox->GetChild(0)))
            {
                childrenSplitViews.Clear();
				tabbedDockWindows = lastSplitView->tabbedDockWindows;

                splitBox->RemoveAllChildren();
                splitBox->Direction(lastSplitView->splitBox->Direction());

                for (int i = 0; i < lastSplitView->splitBox->GetChildCount(); ++i)
                {
					Ref<FWidget> child = lastSplitView->splitBox->GetChild(i);
                    if (child->IsOfType<FDockspaceSplitView>())
                    {
						Ref<FDockspaceSplitView> childSplit = CastTo<FDockspaceSplitView>(child);
						childSplit->parentSplitView = this;
						childSplit->ownerDockspace = ownerDockspace;

                        childrenSplitViews.Add(childSplit);
                    }
					splitBox->AddChild(child.Get());
                }

                UpdateTabs();
            }
		}
    }

    void FDockspaceSplitView::AddDockWindowSplit(FDockingHintPosition splitPosition, Ref<FDockWindow> dockWindow, f32 splitRatio)
    {
        if (!IsSingular() || splitBox->GetChildCount() != 1)
            return;
        if (splitPosition != FDockingHintPosition::Center && !CanBeSplit(dockWindow))
			return;

		splitRatio = Math::Clamp(splitRatio, 0.05f, 0.95f);

        if (splitPosition == FDockingHintPosition::Center)
        {
            AddDockWindow(dockWindow);
            return;
		}

        Array<Ref<FDockWindow>> originalDockWindows = tabbedDockWindows;

    	splitBox->RemoveAllChildren();

        Ref<FDockspaceSplitView> split1, split2;

        splitBox->AddChild(
            FAssignNew(FDockspaceSplitView, split1)
            .FillRatio(1.0f - splitRatio)
        );

        splitBox->AddChild(
            FAssignNew(FDockspaceSplitView, split2)
            .FillRatio(splitRatio)
        );

        childrenSplitViews.AddRange({ split1, split2 });
		split1->parentSplitView = split2->parentSplitView = this;

        switch (splitPosition)
        {
        case FDockingHintPosition::Center: // Do nothing, handled above
	        break;
        case FDockingHintPosition::Left:
            splitBox->Direction(FSplitDirection::Horizontal);
	        for (int i = 0; i < originalDockWindows.GetSize(); ++i)
	        {
				split2->AddDockWindow(originalDockWindows[i]);
	        }
            split1->AddDockWindow(dockWindow);
            split1->VAlign(VAlign::Fill);
            split2->VAlign(VAlign::Fill);
	        break;
        case FDockingHintPosition::Right:
            splitBox->Direction(FSplitDirection::Horizontal);
            for (int i = 0; i < originalDockWindows.GetSize(); ++i)
            {
                split1->AddDockWindow(originalDockWindows[i]);
            }
            split2->AddDockWindow(dockWindow);
            split1->VAlign(VAlign::Fill);
            split2->VAlign(VAlign::Fill);
            break;
        case FDockingHintPosition::Top:
            splitBox->Direction(FSplitDirection::Vertical);
            for (int i = 0; i < originalDockWindows.GetSize(); ++i)
            {
                split2->AddDockWindow(originalDockWindows[i]);
            }
            split1->AddDockWindow(dockWindow);
            split1->HAlign(HAlign::Fill);
            split2->HAlign(HAlign::Fill);
	        break;
        case FDockingHintPosition::Bottom:
            splitBox->Direction(FSplitDirection::Vertical);
            for (int i = 0; i < originalDockWindows.GetSize(); ++i)
            {
                split1->AddDockWindow(originalDockWindows[i]);
            }
            split2->AddDockWindow(dockWindow);
            split1->HAlign(HAlign::Fill);
            split2->HAlign(HAlign::Fill);
	        break;
        }

        tabbedDockWindows.Clear();

        tabWellParent->Enabled(false);

        UpdateTabs();

        SetDockingPreviewEnabled(false, splitPosition);
    }

    void FDockspaceSplitView::AddDockWindowSplit(FDockingHintPosition splitPosition, FDockWindow& dockWindow, f32 splitRatio)
    {
        AddDockWindowSplit(splitPosition, &dockWindow, splitRatio);
    }

    void FDockspaceSplitView::AddDockWindow(Ref<FDockWindow> dockWindow)
    {
        if (!CanBeDocked(dockWindow) || tabbedDockWindows.Exists(dockWindow))
            return;

        tabbedDockWindows.Add(dockWindow);
        tabWell->UpdateTabWell();

        dockWindow->ownerDockspaceSplitView = this;

        if (tabbedDockWindows.GetSize() == 1)
        {
            SetActiveTab(tabWell->GetTabItem(0));
        }

        SetDockingPreviewEnabled(false, FDockingHintPosition::Center);
    }

    int FDockspaceSplitView::GetDockedWindowIndex(Ref<FDockWindow> dockedWindow)
    {
        return tabbedDockWindows.IndexOf(dockedWindow);
    }

    void FDockspaceSplitView::UpdateTabs()
    {
        if (childrenSplitViews.NotEmpty())
        {
            for (Ref<FDockspaceSplitView> childSplitView : childrenSplitViews)
            {
                childSplitView->UpdateTabs();
            }

	        return;
        }

        tabWellParent->Enabled(true);
        tabWell->UpdateTabWell();

        RemoveAllContent();

        if (tabbedDockWindows.IsEmpty())
        {
            if (Ref<FDockspace> dockspace = GetDockspace())
            {
                if (dockspace->DestroyWhenEmpty() && dockspace->GetRootSplit() == this)
                {
                    if (Ref<FFusionContext> context = GetContext())
                    {
                        context->QueueDestroy(); // Destroy the entire window! Be cautious with this!
                    }
                }
            }
            return;
        }

        int activeTabIndex = tabWell->GetTabIndex(selectedTab);
        activeTabIndex = Math::Clamp<int>(activeTabIndex, 0, tabbedDockWindows.GetSize() - 1);

        SetSingleDockWindow(tabbedDockWindows[activeTabIndex].Get());
        tabbedDockWindows[activeTabIndex]->FillRatio(1.0f);
    }

    Ref<FDockTabItem> FDockspaceSplitView::DetachItem(Ref<FDockTabItem> dockTabItem)
    {
        if (!dockTabItem)
            return nullptr;

        if (Ref<FDockspace> dockspace = GetDockspace())
        {
            int index = tabWell->GetTabIndex(dockTabItem);
            if (index < 0 || index >= tabbedDockWindows.GetSize())
                return nullptr;

            Vec2 availSize = GetContext()->GetAvailableSize();

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
                500, 400,
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
            detachedWindow->GetContext()->SetGhostedAvailableSize(availSize);

            PlatformWindow* nativeWindow = detachedWindow->GetPlatformWindow();
            nativeWindow->SetBorderless(true);
            nativeWindow->SetAlwaysOnTop(true);
            nativeWindow->SetOpacity(0.5f);

            Ref<FDockspace> tempDockspace = nullptr;

            detachedWindow->SetWindowContent(
                FAssignNew(FDockspace, tempDockspace)
                .DockspaceType(dockspace->DockspaceType())
                .DestroyWhenEmpty(true)
                .AllowDocking(true)
                .AllowSplitting(dockspace->AllowSplitting())
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                .FillRatio(1.0f)
            );

            tempDockspace->m_OnCreateDockspace = dockspace->m_OnCreateDockspace;
            tempDockspace->detachedDockspaceWindowClass = dockspace->detachedDockspaceWindowClass;
            tempDockspace->m_OnWindowSetup = dockspace->m_OnWindowSetup;

            tempDockspace->originalWindowSize = nativeWindow->GetWindowSize();

            if (Ref<FNativeContext> nativeContext = GetNativeContext())
            {
                tempDockspace->originalWindowSize = nativeContext->GetWindowSize();
            }

            tempDockspace->AddDockWindow(dockWindow);

            Ref<FDockTabItem> newTabItem = tempDockspace->GetRootSplit()->GetTabWell()->GetTabItem(0);
            newTabItem->detached = true;

            FusionApplication::Get()->GetRootContext()->SetFocusWidget(newTabItem.Get());

            if (selectedTab == nullptr && neighborIndex < tabWell->GetTabCount() && neighborIndex >= 0)
            {
                SetActiveTab(tabWell->GetTabItem(neighborIndex));
            }

            detachedWindow->SetContextRecursively(detachedWindow->GetContext().Get());

            if (tabbedDockWindows.IsEmpty())
            {
	            if (Ref<FDockspaceSplitView> parentSplit = this->parentSplitView.Lock())
                {
                    parentSplit->RemoveChildSplitView(this);
				}
            }

            return newTabItem;
        }

        return nullptr;
    }

    bool FDockspaceSplitView::RemoveDockItemInternal(Ref<FDockTabItem> dockTabItem)
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

    bool FDockspaceSplitView::RemoveDockItem(Ref<FDockTabItem> dockTabItem)
    {
        if (!dockTabItem)
            return false;

        if (Ref<FDockspace> dockspace = GetDockspace())
        {
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

            int neighborIndex = index - 1;
            neighborIndex = Math::Clamp<int>(neighborIndex, 0, tabbedDockWindows.GetSize() - 1);

            if (selectedTab == nullptr && neighborIndex < tabWell->GetTabCount() && neighborIndex >= 0)
            {
                SetActiveTab(tabWell->GetTabItem(neighborIndex));
            }

            if (tabbedDockWindows.IsEmpty())
            {
                if (Ref<FDockspaceSplitView> parentSplit = this->parentSplitView.Lock())
                {
                    parentSplit->RemoveChildSplitView(this);
                }
            }

            return true;
        }

        return false;
    }

    void FDockspaceSplitView::OnPaintDockingPreview(FPainter* painter)
    {
    	if (dockingPreviewEnabled)
        {
            FBrush background = FBrush(Color::RGBHex(0x93B8DF).WithAlpha(0.5f));
            Color borderColor = Color::RGBHex(0x5397DB);

            painter->SetBrush(background);
            painter->SetPen(FPen(borderColor, 1));

            Vec2 pos;
            Vec2 size;

            switch (dockingPreviewPosition)
            {
            case FDockingHintPosition::Center:
                size = previewWidget->GetComputedSize();
                break;
            case FDockingHintPosition::Left:
                size = previewWidget->GetComputedSize() * Vec2(0.5f, 1);
                break;
            case FDockingHintPosition::Top:
                size = previewWidget->GetComputedSize() * Vec2(1, 0.5f);
                break;
            case FDockingHintPosition::Right:
                pos = previewWidget->GetComputedSize() * Vec2(0.5f, 0);
                size = previewWidget->GetComputedSize() * Vec2(0.5f, 1);
                break;
            case FDockingHintPosition::Bottom:
                pos = previewWidget->GetComputedSize() * Vec2(0, 0.5f);
                size = previewWidget->GetComputedSize() * Vec2(1, 0.5f);
                break;
            }

            painter->DrawRect(Rect::FromSize(pos, size));
        }
    }

    void FDockspaceSplitView::ApplyStyle()
    {
        Super::ApplyStyle();

        if (Ref<FDockspace> dockspace = GetDockspace())
        {
            dockspace->ApplyStyle();
        }
    }

    bool FDockspaceSplitView::SupportsDropTarget() const
    {
        return IsSingular();
    }

    void FDockspaceSplitView::SetDockingPreviewEnabled(bool enabled, FDockingHintPosition position)
    {
        if (splitBox->GetChildCount() > 1)
            enabled = false;

        dockingPreviewEnabled = enabled;
        dockingPreviewPosition = position;
    }

    Ref<FDockspace> FDockspaceSplitView::GetDockspace()
    {
        if (Ref<FDockspace> dockspace = ownerDockspace.Lock())
        {
            return dockspace;
        }

        Ref<FWidget> widget = GetParent();

        while (widget != nullptr)
        {
            if (widget->IsOfType<FDockspace>())
            {
                ownerDockspace = CastTo<FDockspace>(widget);
                return ownerDockspace.Lock();
            }

            widget = widget->GetParent();
        }

        return nullptr;
    }

    void FDockspaceSplitView::SetSingleDockWindow(Ref<FDockWindow> dockWindow)
    {
        splitBox->RemoveAllChildren();

        splitBox->AddChild(dockWindow.Get());
    }

    void FDockspaceSplitView::RemoveAllContent()
    {
        splitBox->RemoveAllChildren();
    }

    void FDockspaceSplitView::SetGuideVisible(bool visible)
    {
        dockingGuide->Enabled(visible);
    }
}

