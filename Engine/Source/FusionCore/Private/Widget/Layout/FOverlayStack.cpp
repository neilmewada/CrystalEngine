#include "FusionCore.h"

namespace CE
{

    FOverlayStack::FOverlayStack()
    {
        m_ContentHAlign = HAlign::Fill;
        m_ContentVAlign = VAlign::Fill;
    }

    void FOverlayStack::CalculateIntrinsicSize()
    {
        ZoneScoped;

        if (children.IsEmpty())
        {
            Super::CalculateIntrinsicSize();
            return;
        }

        intrinsicSize.width = m_Padding.left + m_Padding.right;
        intrinsicSize.height = m_Padding.top + m_Padding.bottom;

        Vec2 contentSize = {};

        for (const auto& child : children)
        {
            if (!child->Enabled())
                continue;

            child->CalculateIntrinsicSize();

            Vec2 childSize = child->GetIntrinsicSize();
            Vec4 childMargin = child->Margin();

            contentSize.width = Math::Max(contentSize.width, childSize.width + childMargin.left + childMargin.right);
            contentSize.height = Math::Max(contentSize.height, childSize.height + childMargin.top + childMargin.bottom);
        }

        intrinsicSize += contentSize;

        intrinsicSize.width = Math::Clamp(intrinsicSize.width,
            m_MinWidth + m_Padding.left + m_Padding.right,
            m_MaxWidth + m_Padding.left + m_Padding.right);

        intrinsicSize.height = Math::Clamp(intrinsicSize.height,
            m_MinHeight + m_Padding.top + m_Padding.bottom,
            m_MaxHeight + m_Padding.top + m_Padding.bottom);
    }

    void FOverlayStack::PlaceSubWidgets()
    {
        ZoneScoped;

	    Super::PlaceSubWidgets();

        if (children.IsEmpty())
        {
            return;
        }

        Vec2 origin = Vec2(m_Padding.left, m_Padding.top);
        
        Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right,
            m_Padding.top + m_Padding.bottom);

        for (const auto& child : children)
        {
            if (!child->Enabled())
                continue;

            CE::VAlign childVAlign = child->VAlign();
            CE::HAlign childHAlign = child->HAlign();

            if (childVAlign == VAlign::Auto)
                childVAlign = m_ContentVAlign;
            if (childHAlign == HAlign::Auto)
                childHAlign = m_ContentHAlign;

            Vec2 childPos;
            Vec2 childSize;
            Vec2 childIntrinsicSize = child->GetIntrinsicSize();
            Vec4 childMargin = child->Margin();

            switch (childVAlign)
            {
            case VAlign::Auto:
            case VAlign::Fill:
                childSize.y = availableSize.y - childMargin.top - childMargin.bottom;
                childPos.y = origin.y + childMargin.top - childMargin.bottom;
	            break;
            case VAlign::Top:
                childSize.y = childIntrinsicSize.y;
                childPos.y = origin.y + childMargin.top - childMargin.bottom;
	            break;
            case VAlign::Center:
                childSize.y = childIntrinsicSize.y;
                childPos.y = origin.y + childMargin.top - childMargin.bottom + (availableSize.y - childSize.y) * 0.5f;
	            break;
            case VAlign::Bottom:
                childSize.y = childIntrinsicSize.y;
                childPos.y = origin.y + childMargin.top - childMargin.bottom + (availableSize.y - childSize.y);
	            break;
            }

            switch (childHAlign)
            {
            case HAlign::Auto:
            case HAlign::Fill:
                childSize.x = availableSize.x - childMargin.left - childMargin.right;
                childPos.x = origin.x + childMargin.left - childMargin.right;
	            break;
            case HAlign::Left:
                childSize.x = childIntrinsicSize.x;
                childPos.x = origin.x + childMargin.left - childMargin.right;
	            break;
            case HAlign::Center:
                childSize.x = childIntrinsicSize.x;
                childPos.x = origin.x + (availableSize.x - childSize.x) * 0.5f + childMargin.left - childMargin.right;
	            break;
            case HAlign::Right:
                childSize.x = childIntrinsicSize.x;
                childPos.x = origin.x + (availableSize.x - childSize.x) + childMargin.left - childMargin.right;
	            break;
            }

            childPos.x = Math::Max(childPos.x, origin.x);
            childPos.y = Math::Max(childPos.y, origin.y);
            
            child->SetComputedPosition(childPos);
            child->SetComputedSize(childSize);

            child->PlaceSubWidgets();
        }
    }

    void FOverlayStack::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        if (children.IsEmpty())
            return;
        
        if (m_ClipChildren)
        {
            painter->PushClipRect(Matrix4x4::Identity(), computedSize);
        }

        for (const auto& child : children)
        {
            if (!child->Enabled() || !child->Visible())
                continue;

            if (child->IsTranslationOnly())
            {
                painter->PushChildCoordinateSpace(child->GetComputedPosition() + child->Translation());
            }
            else
            {
                painter->PushChildCoordinateSpace(child->GetLocalTransform());
            }

            child->OnPaint(painter);

            painter->PopChildCoordinateSpace();
        }

        if (m_ClipChildren)
        {
            painter->PopClipRect();
        }
    }

    void FOverlayStack::Construct()
    {
        
        
    }
    
}

