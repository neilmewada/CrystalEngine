#include "FusionCore.h"

namespace CE
{

    FCompoundWidget::FCompoundWidget()
    {

    }

    void FCompoundWidget::SetParentSurfaceRecursive(Ref<FSurface> surface)
    {
        ZoneScoped;

        Super::SetParentSurfaceRecursive(surface);

        if (m_Child)
        {
            m_Child->SetParentSurfaceRecursive(surface);
        }
    }

    void FCompoundWidget::DetachChild(Ref<FWidget> child)
    {
        ZoneScoped;

        Super::DetachChild(child);

        if (m_Child == child && m_Child != nullptr)
        {
            m_Child->SetParentSurfaceRecursive(nullptr);
            m_Child->SetParentWidget(nullptr);
            m_Child = nullptr;

            MarkLayoutDirty();
        }
    }

    void FCompoundWidget::SetChild(Ref<FWidget> widget)
    {
        ZoneScoped;

        if (m_Child == widget)
            return;

        DetachChild(m_Child);

        if (widget)
        {
            widget->DetachFromParent();
        }

        m_Child = widget;

        if (m_Child)
        {
            m_Child->SetParentWidget(this);

            if (m_Child->GetParentSurface() != GetParentSurface())
            {
                m_Child->SetParentSurfaceRecursive(GetParentSurface());
            }
        }

        MarkLayoutDirty();
    }

    Vec2 FCompoundWidget::MeasureContent(Vec2 availableSize)
    {
        Vec2 baseSize = GetMinimumContentSize();

		if (!m_Child || !m_Child->Enabled())
        {
            return desiredSize = ApplyLayoutConstraints(baseSize);
        }

		FMargin childMargin = m_Child->Margin();

        Vec2 childAvailableSize = Vec2(
            Math::Max(0.0f, availableSize.x - (childMargin.left + childMargin.right + m_Padding.left + m_Padding.right)),
            Math::Max(0.0f, availableSize.y - (childMargin.top + childMargin.bottom + m_Padding.top + m_Padding.bottom))
        );

        Vec2 childSize = m_Child->MeasureContent(childAvailableSize);

        return desiredSize = ApplyLayoutConstraints(Vec2(
            childSize.x + childMargin.left + childMargin.right + m_Padding.left + m_Padding.right,
            childSize.y + childMargin.top + childMargin.bottom + m_Padding.top + m_Padding.bottom
        ));
    }

    void FCompoundWidget::ArrangeContent(Vec2 finalSize)
    {
	    Super::ArrangeContent(finalSize);

		if (!m_Child || !m_Child->Enabled())
            return;

        FMargin childMargin = m_Child->Margin();

        f32 childAreaWidth = Math::Max(0.0f, finalSize.x - m_Padding.left - m_Padding.right - childMargin.left - childMargin.right);
        f32 childAreaHeight = Math::Max(0.0f, finalSize.y - m_Padding.top - m_Padding.bottom - childMargin.top - childMargin.bottom);

        Vec2 childPos = Vec2(m_Padding.left + childMargin.left, m_Padding.top + childMargin.top);
        Vec2 childSize;

	    switch (m_Child->HAlign())
	    {
	    case HAlign::Auto:
	    case HAlign::Fill:
			childSize.x = childAreaWidth;
		    break;
	    case HAlign::Left:
			childSize.x = Math::Min(m_Child->GetDesiredSize().x, childAreaWidth);
		    break;
	    case HAlign::Center:
			childSize.x = Math::Min(m_Child->GetDesiredSize().x, childAreaWidth);
            childPos.x += (childAreaWidth - childSize.x) / 2.0f;
		    break;
	    case HAlign::Right:
			childSize.x = Math::Min(m_Child->GetDesiredSize().x, childAreaWidth);
			childPos.x += childAreaWidth - childSize.x;
		    break;
	    }

        switch (m_Child->VAlign())
        {
		case VAlign::Auto:
		case VAlign::Fill:
			childSize.y = childAreaHeight;
			break;
		case VAlign::Top:
			childSize.y = Math::Min(m_Child->GetDesiredSize().y, childAreaHeight);
			break;
		case VAlign::Center:
			childSize.y = Math::Min(m_Child->GetDesiredSize().y, childAreaHeight);
			childPos.y += (childAreaHeight - childSize.y) / 2.0f;
			break;
		case VAlign::Bottom:
			childSize.y = Math::Min(m_Child->GetDesiredSize().y, childAreaHeight);
			childPos.y += childAreaHeight - childSize.y;
			break;
        }

		m_Child->SetLayoutPosition(childPos);
        m_Child->ArrangeContent(childSize);
    }

    FCompoundWidget& FCompoundWidget::Child(FWidget& widget)
    {
        SetChild(&widget);
        return *this;
    }
}

