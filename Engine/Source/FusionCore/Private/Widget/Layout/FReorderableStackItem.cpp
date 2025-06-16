#include "FusionCore.h"

namespace CE
{

    FReorderableStackItem::FReorderableStackItem()
    {

    }

    void FReorderableStackItem::Construct()
    {
        Super::Construct();


    }

    void FReorderableStackItem::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent())
        {
            FMouseEvent* mouseEvent = (FMouseEvent*)event;

        }

        if (event->IsDragEvent())
        {
            FDragEvent* dragEvent = (FDragEvent*)event;

            if (event->type == FEventType::DragBegin)
            {
                dragEvent->draggedWidget = this;
                dragEvent->Consume(this);

                startMousePosX = dragEvent->mousePosition.x;

                if (Ref<FReorderableStack> owner = ownerStack.Lock())
                {
                    owner->activeItem = this;
                }
            }
            else if (event->type == FEventType::DragMove)
            {
                dragEvent->draggedWidget = this;
                dragEvent->Consume(this);

                Vec2 finalPos = GetComputedPosition() + Vec2(dragEvent->mousePosition.x - startMousePosX, 0);
                finalPos.x = Math::Clamp(finalPos.x, 0.0f, GetParent()->GetComputedSize().width - GetComputedSize().width);

                if (Ref<FReorderableStack> owner = ownerStack.Lock())
                {
                    owner->activeItem = this;
                    owner->OnActiveItemDragged(false);
                }

                Translation(finalPos - GetComputedPosition());
            }
            else if (event->type == FEventType::DragEnd)
            {
                if (Ref<FReorderableStack> owner = ownerStack.Lock())
                {
                    owner->activeItem = nullptr;
                    owner->OnActiveItemDragged(true);
                }

                Translation(Vec2());
            }
        }

        Super::HandleEvent(event);
    }

    void FReorderableStackItem::OnAttachedToParent(FWidget* parent)
    {
        Super::OnAttachedToParent(parent);

        if (parent && parent->IsOfType<FReorderableStack>())
        {
            ownerStack = (FReorderableStack*)parent;
        }
    }
}

