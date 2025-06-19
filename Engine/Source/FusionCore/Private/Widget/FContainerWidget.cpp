#include "FusionCore.h"

namespace CE
{

    FContainerWidget::FContainerWidget()
    {
        m_ClipChildren = false;
        m_DebugColor = Color::Clear();
    }

    Ref<FWidget> FContainerWidget::FindChildByName(const CE::Name& name, SubClass<FWidget> widgetClass)
    {
        for (int i = 0; i < GetChildCount(); ++i)
        {
            Ref<FWidget> child = GetChild(i);
            if (child->GetName() == name && child->IsOfType(widgetClass))
            {
                return child;
            }
        }

        return nullptr;
    }

    void FContainerWidget::SetContextRecursively(FFusionContext* context)
    {
	    Super::SetContextRecursively(context);

        for (const auto& child : children)
        {
            child->SetContextRecursively(context);
        }
    }

    FWidget* FContainerWidget::HitTest(Vec2 localMousePos)
    {
	    FWidget* thisHitTest = Super::HitTest(localMousePos);
        if (thisHitTest == nullptr)
            return nullptr;
        if (children.IsEmpty())
            return thisHitTest;

        Vec3 invScale = Vec3(1 / m_Scale.x, 1 / m_Scale.y, 1);

	    Vec2 transformedMousePos = (Matrix4x4::Translation(computedSize * m_Anchor) *
			    Matrix4x4::Angle(-m_Angle) *
			    Matrix4x4::Scale(invScale) *
			    Matrix4x4::Translation(-computedPosition - m_Translation - computedSize * m_Anchor)) *
		    Vec4(localMousePos.x, localMousePos.y, 0, 1);
        
	    for (int i = children.GetSize() - 1; i >= 0; --i)
	    {
            Ref<FWidget> child = children[i].Get();
            if (!child->Enabled())
                continue;

            FWidget* result = child->HitTest(transformedMousePos);
            if (result)
            {
                return result;
            }
	    }

        return thisHitTest;
    }

    bool FContainerWidget::ChildExistsRecursive(FWidget* child)
    {
        if (this == child)
            return true;

        for (const auto& widget : children)
        {
	        if (widget->ChildExistsRecursive(child))
                return true;
        }

        return false;
    }

    void FContainerWidget::ApplyStyleRecursively()
    {
	    Super::ApplyStyleRecursively();

        for (const auto& widget : children)
        {
            widget->ApplyStyleRecursively();
        }
    }

    void FContainerWidget::HandleEvent(FEvent* event)
    {
        if (event->stopPropagation)
            return;

        if (event->direction == FEventDirection::TopToBottom)
        {
            for (const auto& child : children)
            {
                if (!child->Enabled())
                    continue;

                child->HandleEvent(event);

                if (event->stopPropagation)
                {
                    event->stopPropagation = false;
                }
            }
        }

        Super::HandleEvent(event);
    }

    void FContainerWidget::InsertChild(int index, FWidget* child)
    {
        if (AddChild(child))
        {
            children.Remove(child);
            children.InsertAt(index, child);
        }
    }

    void FContainerWidget::RemoveChildAt(int index)
    {
        if (index < 0 || index >= children.GetSize())
        {
            RemoveChild(children[index].Get());
        }
    }

    void FContainerWidget::MoveChildToIndex(FWidget* child, int index)
    {
        int curIndex = children.IndexOf(child);
        if (curIndex < 0 || index < 0 || index >= children.GetSize())
            return;

        children.RemoveAt(curIndex);
        children.InsertAt(index, child);

        MarkLayoutDirty();
    }

    void FContainerWidget::DestroyAllChildren()
    {
	    for (int i = children.GetSize() - 1; i >= 0; --i)
	    {
            Ref<FWidget> child = children[i].Lock();
	        if (child == nullptr)
	            continue;
            RemoveChild(child.Get());
            child->BeginDestroy();
	    }
    }

    void FContainerWidget::QueueDestroyAllChildren()
    {
        for (int i = children.GetSize() - 1; i >= 0; --i)
        {
            Ref<FWidget> child = children[i].Lock();
            if (child == nullptr)
                continue;
            RemoveChild(child.Get());
            child->QueueDestroy();
        }
    }

    void FContainerWidget::RemoveAllChildren()
    {
        for (int i = children.GetSize() - 1; i >= 0; --i)
        {
            Ref<FWidget> child = children[i].Lock();
            if (child == nullptr)
                continue;
            RemoveChild(child.Get());
        }
    }

    void FContainerWidget::OnPaint(FPainter* painter)
    {
        if (m_DebugColor.a > 0.001f)
        {
            painter->SetPen(FPen());
            painter->SetBrush(m_DebugColor);

            painter->DrawRect(Rect::FromSize(Vec2(), computedSize));
        }

	    Super::OnPaint(painter);
    }

    void FContainerWidget::ClearStyle()
    {
	    Super::ClearStyle();

        for (const auto& child : children)
        {
            child->ClearStyle();
        }
    }

    bool FContainerWidget::TryAddChild(FWidget* child)
    {
        if (children.Exists(child))
            return false;

        children.Add(child);

        MarkLayoutDirty();
        return true;
    }

    bool FContainerWidget::TryRemoveChild(FWidget* child)
    {
        if (!child)
            return false;
        
        int index = children.IndexOf(child);
        if (index < 0)
            return false;

        children.RemoveAt(index);

        MarkLayoutDirty();
    	return true;
    }

    void FContainerWidget::OnChildWidgetDestroyed(FWidget* child)
    {
	    Super::OnChildWidgetDestroyed(child);
        
        TryRemoveChild(child);
    }

} // namespace CE
