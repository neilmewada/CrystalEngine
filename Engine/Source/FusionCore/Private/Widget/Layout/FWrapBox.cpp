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

        Vec2 contentSize = {};
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
                curPos.x += childSize.width + m_Gap.width;
                curSize.height = Math::Max(curSize.height, childSize.height);
            }
            else if (m_WrapDirection == FWrapBoxDirection::Vertical)
            {
                // TODO:
            }
        }
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
        f32 crossAxisSize = 0;
        f32 remainingSize = 0;
        Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right,
            m_Padding.top + m_Padding.bottom);

        for (const auto& childWeakRef : children)
        {
            auto child = childWeakRef.Lock();
            if (child.IsNull())
                continue;

            if (!child->Enabled())
                continue;


        }
    }
}

