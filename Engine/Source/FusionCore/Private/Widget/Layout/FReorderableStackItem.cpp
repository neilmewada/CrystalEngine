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
            dragging = true;

            if (event->type == FEventType::DragBegin)
            {
                dragEvent->draggedWidget = this;
                dragEvent->Consume(this);

                startMousePosX = dragEvent->mousePosition.x;
                lastMousePosX = startMousePosX;

                dragStartPosX = GetComputedPosition().x;

                if (Ref<FReorderableStack> owner = ownerStack.Lock())
                {
                    owner->activeItem = this;
                }
            }
            else if (event->type == FEventType::DragMove)
            {
                dragEvent->draggedWidget = this;
                dragEvent->Consume(this);

                lastMousePosX = dragEvent->mousePosition.x;
                f32 finalPosX = dragStartPosX + dragEvent->mousePosition.x - startMousePosX;
                finalPosX = Math::Clamp(finalPosX, 0.0f, GetParent()->GetComputedSize().width - GetComputedSize().width);
                f32 thisStart = finalPosX;
                f32 thisEnd = thisStart + GetComputedSize().width;

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

                            if (thisStart >= childPos.x && thisStart <= childPos.x + childSize.x)
                            {
                                if (thisStart < childCenter && thisIndex > i)
                                {
                                    owner->RemoveChild(this);
                                    owner->InsertChild(i, this);

                                    owner->OnItemsRearranged();
                                    break;
                                }
                            }

                            if (thisEnd >= childPos.x && thisEnd <= childPos.x + childSize.x)
                            {
                                if (thisEnd > childCenter && thisIndex < i)
                                {
                                    owner->RemoveChild(this);
                                    owner->InsertChild(i, this);

                                    owner->OnItemsRearranged();
                                    break;
                                }
                            }
                        }
                    }

                    owner->OnActiveItemDragged(dragEvent);
                }

                Translation(Vec2(finalPosX - GetComputedPosition().x, 0));
            }
            else if (event->type == FEventType::DragEnd)
            {
                dragging = false;

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

    void FReorderableStackItem::OnPostComputeLayout()
    {
        Super::OnPostComputeLayout();

        if (dragging)
        {
            f32 finalPosX = dragStartPosX + lastMousePosX - startMousePosX;
            finalPosX = Math::Clamp(finalPosX, 0.0f, GetParent()->GetComputedSize().width - GetComputedSize().width);

            Translation(Vec2(finalPosX - GetComputedPosition().x, 0));
        }
    }
}

