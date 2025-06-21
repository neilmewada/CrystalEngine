#include "FusionCore.h"

namespace CE
{

    FDockTabItem::FDockTabItem()
    {
        
    }

    void FDockTabItem::Construct()
    {
        Super::Construct();

        Child(
            FNew(FHorizontalStack)
            .ContentVAlign(VAlign::Center)
            .ContentHAlign(HAlign::Left)
            .Padding(Vec4(2, 1, 2, 1) * 7.5f)
            (
                FAssignNew(FLabel, tabTitle)
                .FontSize(11)
            )
        );
    }

    void FDockTabItem::HandleEvent(FEvent* event)
    {
        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            if (event->IsMouseEvent())
            {
            	if (event->type == FEventType::MouseEnter)
                {
                    isHovered = true;
                    ApplyStyle();
                }
                else if (event->type == FEventType::MouseLeave)
                {
                    isHovered = false;
                    ApplyStyle();
                }
                else if (event->type == FEventType::MousePress)
                {
                    SetActiveTab();
                }
            }

            if (event->IsDragEvent())
            {
                FDragEvent* dragEvent = (FDragEvent*)event;

                if (dragEvent->type == FEventType::DragMove && joined)
                {
                    startMousePos = dragEvent->mousePosition;
                    lastMousePos = startMousePos;

                    dragStartPosX = GetComputedPosition().x;

                    if (Ref<FFusionContext> context = GetContext())
                    {
                        dragStartPosX += dragEvent->mousePosition.x - GetGlobalPosition().x - GetComputedSize().width / 2.0f;
                    }

                    if (Ref<FReorderableStack> owner = ownerStack.Lock())
                    {
                        owner->SetActiveItem(this);
                    }

                    joined = false;
                }
                else if (dragEvent->type == FEventType::DragMove && detached)
                {
                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    if (Ref<FNativeContext> nativeContext = GetNativeContext())
                    {
                        Vec2 windowPos = nativeContext->GlobalToScreenSpacePosition(dragEvent->mousePosition) - Vec2(30, 20);
                        nativeContext->SetWindowPosition(windowPos.ToVec2i());
                    }

                    Ref<FDockspace> thisDockspace = tabWell->GetDockspace();

                    if (thisDockspace && dragEvent->dropTarget && dragEvent->dropTarget->IsOfType<FDockTabWell>() && dragEvent->dropTarget != tabWell)
                    {
                        Ref<FDockTabWell> dropTabWell = CastTo<FDockTabWell>(dragEvent->dropTarget);
                        Ref<FDockspaceSplitView> dropDockspaceSplitView = dropTabWell->GetDockspaceSplitView();

                        int index = tabWell->GetTabIndex(this);
						Ref<FFusionContext> thisContext = GetContext();

                        if (thisContext && dropDockspaceSplitView && index >= 0)
                        {
	                        if (Ref<FDockWindow> dockWindow = thisDockspace->GetRootSplit()->GetTabbedDockWindow(index))
                            {
                                Ref<FFusionContext> dropContext = dropDockspaceSplitView->GetContext();

                                if (dropContext && dropDockspaceSplitView->CanBeDocked(dockWindow))
                                {
                                    thisDockspace->GetRootSplit()->RemoveDockItem(this);

                                    Vec2 screenSpacePos = thisContext->GlobalToScreenSpacePosition(dragEvent->mousePosition);
									Vec2 dropContextSpacePos = dropContext->ScreenToGlobalSpacePosition(screenSpacePos);

									Vec2 localDropPos = dropTabWell->GetGlobalTransform() * Vec4(dropContextSpacePos.x, dropContextSpacePos.y, 0.0f, 1.0f);

                                    //CE_LOG(Info, All, "Drop Pos: {}", localDropPos);

                                    dropDockspaceSplitView->AddDockWindow(dockWindow);

                                    int tabIndex = dropDockspaceSplitView->GetDockedWindowIndex(dockWindow);

                                    if (Ref<FDockTabItem> newTabItem = dropDockspaceSplitView->GetDockTabItem(tabIndex))
                                    {
                                        dropDockspaceSplitView->SetActiveTab(newTabItem);

                                        newTabItem->joined = true;

                                        dragEvent->draggedWidget = newTabItem.Get();

                                        newTabItem->Focus();

                                        if (Ref<FDockspaceSplitView> guideDockspaceLock = guideDockspaceSplitView.Lock())
                                        {
                                            guideDockspaceLock->SetGuideVisible(false);
                                            guideDockspaceSplitView = nullptr;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (thisDockspace && dragEvent->dropTarget && dragEvent->dropTarget->IsOfType<FDockspaceSplitView>())
                    {
                        Ref<FDockspaceSplitView> dropDockspaceSplitView = CastTo<FDockspaceSplitView>(dragEvent->dropTarget);
                        if (Ref<FDockspace> dropDockspace = dropDockspaceSplitView->GetDockspace())
                        {
                            int index = tabWell->GetTabIndex(this);

                            if (index >= 0)
                            {
                                if (Ref<FDockWindow> dockWindow = thisDockspace->GetRootSplit()->GetTabbedDockWindow(index))
                                {
                                    if (guideDockspaceSplitView != dropDockspaceSplitView && dropDockspace->AllowSplitting() && 
                                        dropDockspaceSplitView->CanBeDocked(dockWindow) && dropDockspaceSplitView->CanBeSplit(dockWindow))
                                    {
                                        if (guideDockspaceSplitView)
                                        {
                                            guideDockspaceSplitView->SetGuideVisible(false);
                                        }

                                        dropDockspaceSplitView->SetGuideVisible(true);
                                        guideDockspaceSplitView = dropDockspaceSplitView;
                                    }
                                }
                            }
                        }
                    }
                    else if (dragEvent->dropTarget == nullptr || !dragEvent->dropTarget->IsOfType<FDockingHint>())
                    {
                        if (Ref<FDockspaceSplitView> guideDockspaceSplitViewLock = guideDockspaceSplitView.Lock())
                        {
                            guideDockspaceSplitViewLock->SetGuideVisible(false);
                            guideDockspaceSplitView = nullptr;
                        }
                    }
                }
                else if (dragEvent->type == FEventType::DragEnd && detached) // Detached items are always temporary, i.e. they have only 1 root split view and 1 child FDockWindow
                {
                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    detached = false;

                    Ref<FDockspaceSplitView> splitInDockspaceView = nullptr;
                    FDockingHintPosition splitPosition = FDockingHintPosition::Center;

                    if (Ref<FDockspaceSplitView> guideDockspaceSplitViewLock = guideDockspaceSplitView.Lock())
                    {
                        if (guideDockspaceSplitViewLock->IsDockingPreviewEnabled())
                        {
                            splitInDockspaceView = guideDockspaceSplitViewLock;
                            splitPosition = guideDockspaceSplitViewLock->GetDockingPreviewHintPosition();
                        }
                        guideDockspaceSplitViewLock->SetGuideVisible(false);
                        guideDockspaceSplitView = nullptr;
                    }

                    if (splitInDockspaceView)
                    {
                        if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
                        {
                            if (Ref<FNativeContext> nativeContext = GetNativeContext())
                            {
                                if (Ref<FDockWindow> thisDockWindow = dockspace->GetRootSplit()->GetTabbedDockWindow(this))
                                {
                                    nativeContext->SetGhosted(false);

                                    if (splitInDockspaceView->CanBeSplit(thisDockWindow))
                                    {
                                        Ref<FDockTabWell> dropTabWell = splitInDockspaceView->GetTabWell();

                                        dockspace->GetRootSplit()->RemoveDockItem(this);

                                        if (splitPosition == FDockingHintPosition::Center)
                                        {
                                            splitInDockspaceView->AddDockWindow(thisDockWindow);

                                            int tabIndex = splitInDockspaceView->GetDockedWindowIndex(thisDockWindow);

                                            if (Ref<FDockTabItem> newTabItem = splitInDockspaceView->GetDockTabItem(tabIndex))
                                            {
                                                splitInDockspaceView->SetActiveTab(newTabItem);

                                                newTabItem->joined = true;

                                                dragEvent->draggedWidget = newTabItem.Get();

                                                newTabItem->Focus();

                                                if (Ref<FDockspaceSplitView> guideDockspaceLock = guideDockspaceSplitView.Lock())
                                                {
                                                    guideDockspaceLock->SetGuideVisible(false);
                                                    guideDockspaceSplitView = nullptr;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            splitInDockspaceView->AddDockWindowSplit(splitPosition, thisDockWindow);

                                            if (Ref<FDockspaceSplitView> guideDockspaceLock = guideDockspaceSplitView.Lock())
                                            {
                                                guideDockspaceLock->SetGuideVisible(false);
                                                guideDockspaceSplitView = nullptr;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
                    {
                        if (Ref<FNativeContext> nativeContext = GetNativeContext())
                        {
                            nativeContext->SetGhosted(false);

                            if (Ref<FDockWindow> thisDockWindow = dockspace->GetRootSplit()->GetTabbedDockWindow(this))
                            {
	                            PlatformWindow* nativeWindow = nativeContext->GetPlatformWindow();
	                            Vec2i originalPos = nativeWindow != nullptr ? nativeWindow->GetWindowPosition() : Vec2i();

                                auto onCreateDockspace = dockspace->OnCreateDockspace();
                                
                                dockspace->GetRootSplit()->RemoveDockItem(this);

                                Ref<FDockspaceWindow> newWindow = FusionApplication::Get()->CreateNativeWindow<FDockspaceWindow>(Title(), Title(),
                                    dockspace->originalWindowSize.width,
                                    dockspace->originalWindowSize.height,
                                    dockspace->detachedDockspaceWindowClass,
                                    {
                                        .maximised = false,
                                        .fullscreen = false,
                                        .resizable = true,
                                        .hidden = false,
                                        .openCentered = nativeWindow == nullptr,
                                        .openPos = originalPos,
                                        .windowFlags = PlatformWindowFlags::DestroyOnClose
                                    });

                                newWindow->GetContext()->SetGhosted(false);

                                Ref<FDockspace> newDockspace = newWindow->GetDockspace();

                                newDockspace->DestroyWhenEmpty(true);

                                newDockspace->m_OnCreateDockspace = onCreateDockspace;
                                newDockspace->m_OnWindowSetup = dockspace->m_OnWindowSetup;

                                if (newDockspace->m_OnWindowSetup.IsValid())
                                {
                                    newDockspace->m_OnWindowSetup.Invoke(newWindow, this);
                                }

                                newDockspace->originalWindowSize = dockspace->originalWindowSize;

                                newDockspace->AddDockWindow(thisDockWindow);

                                PlatformWindow* newNativeWindow = newWindow->GetPlatformWindow();
                                newNativeWindow->SetBorderless(true);
                                newNativeWindow->SetWindowPosition(originalPos);

                                newWindow->SetContextRecursively(newWindow->GetContext().Get());
                            }
                        }
                    }
                }
            }
        }

        Super::HandleEvent(event);
    }

    bool FDockTabItem::CanBeDetached()
    {
        if (!canBeDetached)
            return false;

        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            if (Ref<FDockspaceSplitView> dockspaceSplitView = tabWell->GetDockspaceSplitView())
            {
                return dockspaceSplitView->CanDetach(this);
            }
        }

        return false;
    }

    Ref<FReorderableStackItem> FDockTabItem::DetachItem()
    {
        if (!CanBeDetached())
            return nullptr;

        detached = false;

        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            if (Ref<FDockspaceSplitView> dockspaceSplitView = tabWell->GetDockspaceSplitView())
            {
	            if (Ref<FDockTabItem> detachedTabItem = dockspaceSplitView->DetachItem(this))
	            {
					return detachedTabItem;
	            }
            }

            if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
            {
                if (Ref<FDockTabItem> detachedTabItem = dockspace->GetRootSplit()->DetachItem(this))
                {
                    return detachedTabItem;
                }
            }
        }

        return nullptr;
    }

    bool FDockTabItem::SupportsDragEvents() const
    {
        return true;
    }

    void FDockTabItem::SetActiveTab()
    {
        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            tabWell->SetActiveTab(this);
        }
    }

    void FDockTabItem::ApplyStyle()
    {
        Super::ApplyStyle();

        if (Ref<FDockTabWell> tabWell = owner.Lock())
        {
            tabWell->ApplyStyle();
        }
    }
}

