#include "FusionCore.h"

namespace CE
{

    FCompoundWidget::FCompoundWidget()
    {

    }

    void FCompoundWidget::CalculateIntrinsicSize()
    {
        intrinsicSize = Vec2(m_MinWidth + m_Padding.left + m_Padding.right,
            m_MinHeight + m_Padding.top + m_Padding.bottom);

        if (!m_Child || !m_Child->Enabled())
            return;

        if (GetName() == "DebugTreeView")
        {
            String::IsAlphabet('a');
        }

        m_Child->CalculateIntrinsicSize();

        Vec2 childSize = m_Child->GetIntrinsicSize();
        const Vec4& childMargin = m_Child->Margin();

        intrinsicSize.width = Math::Max(intrinsicSize.width, childSize.width + m_Padding.left + m_Padding.right + childMargin.left + childMargin.right);
        intrinsicSize.height = Math::Max(intrinsicSize.height, childSize.height + m_Padding.top + m_Padding.bottom + childMargin.top + childMargin.bottom);

        ApplyIntrinsicSizeConstraints();
    }

    void FCompoundWidget::PlaceSubWidgets()
    {
        Super::PlaceSubWidgets();

        if (!m_Child || !m_Child->Enabled())
            return;

        if (GetName() == "DebugTreeView")
        {
            String::IsAlphabet('a');
        }

        Vec4 childMargin = m_Child->Margin();
        Vec2 childIntrinsicSize = m_Child->GetIntrinsicSize();
        Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right + childMargin.left + childMargin.right,
            m_Padding.top + m_Padding.bottom + childMargin.top + childMargin.bottom);

        m_Child->computedPosition = Vec2(m_Padding.left + childMargin.left, m_Padding.top + childMargin.top);
        m_Child->computedSize = computedSize -
            Vec2(m_Padding.left + m_Padding.right + childMargin.left + childMargin.right,
                m_Padding.top + m_Padding.bottom + childMargin.top + childMargin.bottom);

        CE::VAlign childVAlign = m_Child->VAlign();
        CE::HAlign childHAlign = m_Child->HAlign();

        switch (childVAlign)
        {
        case VAlign::Auto:
        case VAlign::Fill:
            m_Child->computedSize.height = computedSize.height - (m_Padding.top + m_Padding.bottom + childMargin.top + childMargin.bottom);
            m_Child->computedPosition.y = m_Padding.top + childMargin.top;
	        break;
        case VAlign::Top:
            m_Child->computedSize.height = childIntrinsicSize.height;
            m_Child->computedPosition.y = m_Padding.top + childMargin.top;
	        break;
        case VAlign::Center:
            m_Child->computedSize.height = childIntrinsicSize.height;
            m_Child->computedPosition.y = m_Padding.top + childMargin.top + (availableSize.height - childIntrinsicSize.height) * 0.5f;
	        break;
        case VAlign::Bottom:
            m_Child->computedSize.height = childIntrinsicSize.height;
            m_Child->computedPosition.y = m_Padding.top + childMargin.top + (availableSize.height - childIntrinsicSize.height);
	        break;
        }

        switch (childHAlign)
        {
        case HAlign::Auto:
        case HAlign::Fill:
            m_Child->computedSize.width = computedSize.width - (m_Padding.left + m_Padding.right + childMargin.left + childMargin.right);
            m_Child->computedPosition.x = m_Padding.left + childMargin.left;
            break;
        case HAlign::Left:
            m_Child->computedSize.width = childIntrinsicSize.width;
            m_Child->computedPosition.x = m_Padding.left + childMargin.left;
            break;
        case HAlign::Center:
            m_Child->computedSize.width = childIntrinsicSize.width;
            m_Child->computedPosition.x = m_Padding.left + childMargin.left + (availableSize.width - childIntrinsicSize.width) * 0.5f;
            break;
        case HAlign::Right:
            m_Child->computedSize.width = childIntrinsicSize.width;
            m_Child->computedPosition.x = m_Padding.left + childMargin.left + (availableSize.width - childIntrinsicSize.width);
            break;
        }

        m_Child->PlaceSubWidgets();

        OnPostComputeLayout();
    }

    void FCompoundWidget::OnPaint(FPainter* painter)
    {
        ZoneScoped;

	    Super::OnPaint(painter);

        if (!isCulled && m_Child && m_Child->Enabled() && m_Child->Visible())
        {
            if (m_ClipChildren)
            {
                painter->PushClipRect(Matrix4x4::Identity(), computedSize);
            }

            if (m_Child->isTranslationOnly)
            {
	            painter->PushChildCoordinateSpace(m_Child->computedPosition + m_Child->m_Translation);
            }
            else
            {
	            painter->PushChildCoordinateSpace(m_Child->GetLocalTransform());
            }

            m_Child->OnPaint(painter);

            painter->PopChildCoordinateSpace();

            if (m_ClipChildren)
            {
                painter->PopClipRect();
            }
        }
    }

    void FCompoundWidget::SetContextRecursively(FFusionContext* context)
    {
	    Super::SetContextRecursively(context);

        if (m_Child)
        {
            m_Child->SetContextRecursively(context);
        }
    }

    FWidget* FCompoundWidget::HitTest(Vec2 localMousePos)
    {
	    FWidget* thisHitTest = Super::HitTest(localMousePos);
        if (!thisHitTest)
            return nullptr;
        if (!m_Child || !m_Child->Enabled())
            return thisHitTest;

        auto invScale = Vec3(1 / m_Scale.x, 1 / m_Scale.y, 1.0f);

        Vec2 transformedMousePos = (Matrix4x4::Translation(computedSize * m_Anchor) *
            Matrix4x4::Angle(-m_Angle) *
            Matrix4x4::Scale(invScale) *
            Matrix4x4::Translation(-computedPosition - m_Translation - computedSize * m_Anchor)) *
            Vec4(localMousePos.x, localMousePos.y, 0, 1);

        FWidget* result = m_Child->HitTest(transformedMousePos);
        if (result != nullptr)
            return result;
        return thisHitTest;
    }

    bool FCompoundWidget::ChildExistsRecursive(FWidget* child)
    {
	    if (this == child)
            return true;

        return m_Child != nullptr && m_Child->ChildExistsRecursive(child);
    }

    void FCompoundWidget::ApplyStyleRecursively()
    {
	    Super::ApplyStyleRecursively();

        if (m_Child)
        {
            m_Child->ApplyStyleRecursively();
        }
    }

    void FCompoundWidget::HandleEvent(FEvent* event)
    {
        if (event->stopPropagation)
            return;

        if (m_Child && m_Child->Enabled() && event->direction == FEventDirection::TopToBottom)
        {
            m_Child->HandleEvent(event);
        }

	    Super::HandleEvent(event);
    }

    void FCompoundWidget::ClearStyle()
    {
	    Super::ClearStyle();

        if (m_Child)
        {
            m_Child->ClearStyle();
        }
    }

    void FCompoundWidget::RemoveChildWidget()
    {
        if (m_Child)
        {
            RemoveChild(m_Child.Get());
        }
    }

    bool FCompoundWidget::TryAddChild(FWidget* child)
    {
        if (!child)
            return false;

        if (m_Child)
        {
            RemoveChild(m_Child.Get());
        }

        m_Child = child;
        child->parent = this;
        MarkLayoutDirty();
        return true;
    }

    bool FCompoundWidget::TryRemoveChild(FWidget* child)
    {
	    if (!child || child != m_Child.Get())
            return false;

        m_Child = nullptr;
        child->parent = nullptr;
        MarkLayoutDirty();
        return true;
    }

    void FCompoundWidget::OnChildWidgetDestroyed(FWidget* child)
    {
	    Super::OnChildWidgetDestroyed(child);

        if (child == m_Child.Get())
        {
            TryRemoveChild(child);
        }
    }

}

