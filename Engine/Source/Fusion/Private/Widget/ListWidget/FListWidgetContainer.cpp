#include "Fusion.h"

namespace CE
{

    FListWidgetContainer::FListWidgetContainer()
    {

    }

    FListWidgetContainer::~FListWidgetContainer()
    {
    }

    void FListWidgetContainer::CalculateIntrinsicSize()
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

            if (!child->IsCulled() || !child->IntrinsicSizeExists())
            {
                child->CalculateIntrinsicSize();
            }

            Vec2 childSize = child->GetIntrinsicSize();
            Vec4 childMargin = child->Margin();

            contentSize.height += childSize.height + childMargin.top + childMargin.bottom;
            contentSize.width = Math::Max(contentSize.width, childSize.width + childMargin.left + childMargin.right);
        }

        contentSize.height += Math::Max<f32>(0, m_Gap * (children.GetSize() - 1));

        intrinsicSize.width += contentSize.width;
        intrinsicSize.height += contentSize.height;

        ApplyIntrinsicSizeConstraints();
    }

    void FListWidgetContainer::PlaceSubWidgets()
    {
        ZoneScoped;

	    Super::PlaceSubWidgets();

        if (children.IsEmpty())
        {
            return;
        }

        Vec2 curPos = Vec2(m_Padding.left, m_Padding.top);
        Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right,
            m_Padding.top + m_Padding.bottom);

        if (children.GetSize() > 1)
        {
            availableSize.height -= m_Gap * (f32)(children.GetSize() - 1);
        }

        f32 remainingSize = availableSize.height;

        for (const auto& child : children)
        {
            if (!child->Enabled())
                continue;

            Vec2 childIntrinsicSize = child->GetIntrinsicSize();

            child->SetComputedPosition(curPos + Vec2(child->Margin().left, child->Margin().top));

            Vec2 childSize = Vec2(availableSize.width, childIntrinsicSize.height);
            child->SetComputedSize(childSize);

            child->ApplySizeConstraints();

            curPos.y += child->GetComputedSize().height + child->Margin().bottom + m_Gap;

            if (!child->IsCulled())
            {
                child->PlaceSubWidgets();
            }
        }
    }

    void FListWidgetContainer::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);

        if (children.IsEmpty() || !Enabled())
            return;

        painter->PushChildCoordinateSpace(localTransform);
        if (m_ClipChildren)
        {
            painter->PushClipRect(Matrix4x4::Identity(), computedSize);
        }

        for (const auto& child : children)
        {
            if (!child->Enabled())
                continue;

            child->OnPaint(painter);
        }

        if (m_ClipChildren)
        {
            painter->PopClipRect();
        }
        painter->PopChildCoordinateSpace();
    }

}

