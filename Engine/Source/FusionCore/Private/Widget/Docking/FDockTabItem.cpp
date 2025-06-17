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

                if (dragEvent->type == FEventType::DragMove && detached)
                {
                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    if (Ref<FNativeContext> nativeContext = GetNativeContext())
                    {
	                    if (PlatformWindow* nativeWindow = nativeContext->GetPlatformWindow())
	                    {
                            if (isFirstDrag)
                            {
                                isFirstDrag = false;

                                f32 scaling = PlatformApplication::Get()->GetSystemDpi() / 96.0f;
#if PLATFORM_MAC
                                scaling = 1;
#elif PLATFORM_LINUX
                                scaling *= FusionApplication::Get()->GetDefaultScalingFactor();
#endif

                                Vec2 globalPos = GetGlobalPosition();

                                Vec2 tabItemScreenPos = nativeContext->GlobalToScreenSpacePosition(globalPos);
                                Vec2 tabItemScreenSize = GetComputedSize();

                                nativeWindow->SetWindowPosition((tabItemScreenPos - globalPos * scaling + dragEvent->mousePosition).ToVec2i());
                            }
                            else
                            {
                                f32 scaling = PlatformApplication::Get()->GetSystemDpi() / 96.0f;
#if PLATFORM_MAC
                                scaling = 1;
#elif PLATFORM_LINUX
                                scaling *= FusionApplication::Get()->GetDefaultScalingFactor();
#endif

                                Vec2 newPos = nativeWindow->GetWindowPosition().ToVec2() + (dragEvent->mousePosition - dragEvent->prevMousePosition) * scaling;
                                nativeContext->SetWindowPosition(newPos.ToVec2i());
                            }
	                    }
                    }
                }
                else if (dragEvent->type == FEventType::DragEnd && detached)
                {
                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    detached = false;

                    if (Ref<FNativeContext> nativeContext = GetNativeContext())
                    {
                        if (PlatformWindow* nativeWindow = nativeContext->GetPlatformWindow())
                        {
                            nativeWindow->SetOpacity(1.0f);
                        }
                    }
                }
            }
        }

        Super::HandleEvent(event);
    }

    bool FDockTabItem::CanBeDetached()
    {
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

