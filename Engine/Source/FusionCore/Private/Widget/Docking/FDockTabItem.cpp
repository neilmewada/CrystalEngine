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
                        Ref<FDockspace> dropDockspace = dropTabWell->GetDockspace();

                        int index = tabWell->GetTabIndex(this);

                        if (dropDockspace && index >= 0)
                        {
	                        if (Ref<FDockWindow> dockWindow = thisDockspace->GetTabbedDockWindow(index))
                            {
                                if (dropDockspace->CanBeDocked(dockWindow))
                                {
                                    thisDockspace->RemoveDockItem(this);
                                    dropDockspace->AddDockWindow(dockWindow);

                                    int tabIndex = dropDockspace->GetDockedWindowIndex(dockWindow);

                                    if (Ref<FDockTabItem> newTabItem = dropDockspace->GetDockTabItem(tabIndex))
                                    {
                                        dropDockspace->SetActiveTab(newTabItem);

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

                            if (dropDockspaceSplitView && index >= 0)
                            {
                                if (Ref<FDockWindow> dockWindow = thisDockspace->GetTabbedDockWindow(index))
                                {
                                    if (guideDockspaceSplitView != dropDockspaceSplitView && dropDockspace->AllowSplitting() && dropDockspace->CanBeDocked(dockWindow))
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
                            guideDockspaceSplitViewLock = nullptr;
                        }
                    }
                }
                else if (dragEvent->type == FEventType::DragEnd && detached)
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
                                // TODO
                            }
                        }
                    }
                    else if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
                    {
                        if (Ref<FNativeContext> nativeContext = GetNativeContext())
                        {
                            nativeContext->SetGhosted(false);

                            if (Ref<FDockWindow> thisDockWindow = dockspace->GetTabbedDockWindow(this))
                            {
	                            PlatformWindow* nativeWindow = nativeContext->GetPlatformWindow();
	                            Vec2i originalPos = nativeWindow != nullptr ? nativeWindow->GetWindowPosition() : Vec2i();

                                auto onCreateDockspace = dockspace->OnCreateDockspace();
                                
                                dockspace->RemoveDockItem(this);

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
            if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
            {
                return dockspace->CanDetach(this);
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
            if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
            {
                if (Ref<FDockTabItem> detachedTabItem = dockspace->DetachItem(this))
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

