#include "FusionCore.h"

namespace CE
{

    FDockTabItem::FDockTabItem()
    {
        m_Detachable = true;
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
                FMouseEvent* mouseEvent = (FMouseEvent*)event;

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
                            	thisDockspace->RemoveDockItem(this);
                                dropDockspace->AddDockWindow(dockWindow);

                                int tabIndex = dropDockspace->GetDockedWindowIndex(dockWindow);

	                            if (Ref<FDockTabItem> newTabItem = dropDockspace->GetDockTabItem(tabIndex))
                                {
                                    dropDockspace->SetActiveTab(newTabItem);

                                    newTabItem->joined = true;

                                    dragEvent->draggedWidget = newTabItem.Get();

                                    newTabItem->Focus();
                                }
                            }
                        }
                    }
                }
                else if (dragEvent->type == FEventType::DragEnd && detached)
                {
                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    detached = false;

                    if (Ref<FDockspace> dockspace = tabWell->GetDockspace())
                    {
                        if (Ref<FNativeContext> nativeContext = GetNativeContext())
                        {
                            nativeContext->SetGhosted(false);

                            if (PlatformWindow* nativeWindow = nativeContext->GetPlatformWindow())
                            {
                            	if (Ref<FDockWindow> thisDockWindow = dockspace->GetTabbedDockWindow(this))
                                {
                                    // TODO: Dragging stopped

                                    dockspace->RemoveDockItem(this);

                                    Ref<FWindow> newWindow = FusionApplication::Get()->CreateNativeWindow(Title(), Title(),
                                        dockspace->originalWindowSize.width,
                                        dockspace->originalWindowSize.height,
                                        dockspace->GetDetachedWindowClass(),
                                        {
                                            .maximised = false,
                                            .fullscreen = false,
                                            .resizable = true,
                                            .hidden = false,
											.openCentered = false,
											.openPos = nativeWindow->GetWindowPosition(),
                                            .windowFlags = PlatformWindowFlags::DestroyOnClose
                                        });
                            		
                                    Ref<FDockspace> newDockspace = nullptr;

                                    newWindow->SetWindowContent(
                                        FAssignNew(FDockspace, newDockspace)
                                        .DockspaceType(dockspace->DockspaceType())
                                        .DestroyWhenEmpty(true)
                                        .HAlign(HAlign::Fill)
                                        .VAlign(VAlign::Fill)
                                        .FillRatio(1.0f)
                                    );

                                    if (newWindow->IsOfType<FToolWindow>())
                                    {
                                        Ref<FToolWindow> toolWindow = CastTo<FToolWindow>(newWindow);
                                        toolWindow->ContentPadding(Vec4());
                                        toolWindow->Title(Title());
                                    }

                                    newDockspace->originalWindowSize = dockspace->originalWindowSize;

                                    newDockspace->AddDockWindow(thisDockWindow);

                                    PlatformWindow* newNativeWindow = newWindow->GetPlatformWindow();
                                    newNativeWindow->SetBorderless(true);
                                    newNativeWindow->SetWindowPosition(nativeWindow->GetWindowPosition());

                                    //nativeWindow->SetOpacity(1.0f);
                                    //nativeWindow->SetAlwaysOnTop(false);
                                }
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
        if (!m_Detachable)
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

