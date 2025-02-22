#include "FusionCore.h"

namespace CE
{

    FWrapBox::FWrapBox()
    {
        m_WrapDirection = FWrapBoxDirection::Horizontal;
        m_HAlign = HAlign::Fill;
        m_VAlign = VAlign::Fill;
    }

    void FWrapBox::Construct()
    {
        Super::Construct();


    }

    void FWrapBox::CalculateIntrinsicSize()
    {
        ZoneScoped;

        if (children.IsEmpty())
        {
            Super::CalculateIntrinsicSize();
            return;
        }

        intrinsicSize.width = m_Padding.left + m_Padding.right;
        intrinsicSize.height = m_Padding.top + m_Padding.bottom;

        Vec2 curPos = Vec2(m_Padding.left, m_Padding.top);
        Vec2 curSize = Vec2();
        bool isFinished = false;

        for (const auto& childWeakRef : children)
        {
            auto child = childWeakRef.Lock();
            if (child.IsNull())
                continue;

            if (!child->Enabled())
                continue;

            child->CalculateIntrinsicSize();

            Vec2 childSize = child->GetIntrinsicSize();
            Vec4 childMargin = child->Margin();

            if (m_WrapDirection == FWrapBoxDirection::Horizontal)
            {
                curPos.x += childSize.width + childMargin.left + childMargin.right;
                curSize.height = Math::Max(curSize.height, childSize.height);
            }
            else if (m_WrapDirection == FWrapBoxDirection::Vertical)
            {
                curPos.y += childSize.height + childMargin.top + childMargin.bottom;
                curSize.width = Math::Max(curSize.width, childSize.width);
            }
        }

        if (m_WrapDirection == FWrapBoxDirection::Horizontal)
        {
            curSize.width += m_Gap.width * (children.GetSize() - 1);
        }
        else if (m_WrapDirection == FWrapBoxDirection::Vertical)
        {
            curSize.height += m_Gap.height * (children.GetSize() - 1);
        }

        intrinsicSize.width += curSize.width;
        intrinsicSize.height += curSize.height;

        ApplyIntrinsicSizeConstraints();
    }

    void FWrapBox::PlaceSubWidgets()
    {
        ZoneScoped;

        Super::PlaceSubWidgets();

        if (children.IsEmpty())
        {
            return;
        }

        Vec2 curPos = Vec2(m_Padding.left, m_Padding.top);
        f32 curCrossAxisSize = 0;
        f32 totalCrossAxisSize = 0;
        Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right,
            m_Padding.top + m_Padding.bottom);
        bool fullyCovered = false;

        for (int i = 0; i < children.GetSize(); i++)
        {
            auto child = children[i].Lock();
            if (child.IsNull())
                continue;

            if (!child->Enabled())
                continue;

            child->SetComputedPosition(curPos + child->Margin().min);
            child->SetComputedSize(child->GetIntrinsicSize());

            child->PlaceSubWidgets();
            fullyCovered = false;

            if (m_WrapDirection == FWrapBoxDirection::Horizontal)
            {
                if (child->GetComputedPosition().x + child->GetComputedSize().width > computedSize.width)
                {
                    fullyCovered = true;

                    curCrossAxisSize = Math::Max(curCrossAxisSize, child->GetComputedSize().height);

                    curPos.y += curCrossAxisSize + m_Gap.height;
                    curPos.x = m_Padding.left;

                    child->SetComputedPosition(curPos);

                    totalCrossAxisSize += curCrossAxisSize + m_Gap.height;
                    curCrossAxisSize = child->GetComputedSize().height;

                    if (i == children.GetSize() - 1)
                    {
                        computedSize.height = totalCrossAxisSize + m_Padding.bottom + m_Padding.top;
                    }
                }

                curPos.x += child->GetComputedSize().width + m_Gap.width;

                if (curPos.x > computedSize.width)
                {
                    curPos.y += curCrossAxisSize + m_Gap.height;
                    curPos.x = m_Padding.left;

                    totalCrossAxisSize += curCrossAxisSize + m_Gap.height;
                    curCrossAxisSize = 0;
                }

                curCrossAxisSize = Math::Max(curCrossAxisSize, child->GetComputedSize().height);

                if (!fullyCovered && i == children.GetSize() - 1)
                {
                    totalCrossAxisSize += curCrossAxisSize;
                    computedSize.height = totalCrossAxisSize + m_Padding.bottom + m_Padding.top;
                }
            }
            else if (m_WrapDirection == FWrapBoxDirection::Vertical)
            {
                // TODO
            }
        }
    }

    void FWrapBox::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);

        if (children.IsEmpty() || !Enabled())
            return;

        if (m_ClipChildren)
        {
            Vec3 invScale = Vec3(1 / m_Scale.x, 1 / m_Scale.y, 1);

            painter->PushClipRect(Matrix4x4::Translation(computedPosition + GetComputedSize() * m_Anchor) *
                Matrix4x4::Angle(-m_Angle) *
                Matrix4x4::Scale(invScale) *
                Matrix4x4::Translation(-computedPosition - m_Translation - GetComputedSize() * m_Anchor), computedSize);
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

}

