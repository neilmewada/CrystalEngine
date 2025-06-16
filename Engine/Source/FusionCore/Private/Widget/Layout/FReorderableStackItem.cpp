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
                Vec2 thisSize = GetComputedSize();
                f32 thisCenter = finalPos.x;

                if (Ref<FReorderableStack> owner = ownerStack.Lock())
                {
                    owner->activeItem = this;
                    int thisIndex = owner->children.IndexOf(this);

                    for (int i = 0; i < owner->children.GetSize(); ++i)
                    {
                        if (Ref<FWidget> child = owner->children[i].Lock())
                        {
                            if (child == this)
                                continue;

                            Vec2 childPos = child->GetComputedPosition();
                            Vec2 childSize = child->GetComputedSize();
                            f32 childCenter = childPos.x + childSize.x / 2;

                            if (thisCenter >= childPos.x && thisCenter <= childPos.x + childSize.x)
                            {
                                if (thisCenter < childCenter && thisIndex > i)
                                {
                                    owner->RemoveChild(this);
                                    owner->InsertChild(i, this);
                                    break;
                                }

                                if (thisCenter > childCenter && thisIndex < i)
                                {

                                }
                                // TODO
                            }
                        }
                    }

                    owner->OnActiveItemDragged(dragEvent);
                }

                Translation(finalPos - GetComputedPosition());
            }
            else if (event->type == FEventType::DragEnd)
            {
                if (Ref<FReorderableStack> owner = ownerStack.Lock())
                {
                    owner->activeItem = nullptr;
                    owner->OnActiveItemDragged(dragEvent);
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

