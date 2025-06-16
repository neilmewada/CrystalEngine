#include "FusionCore.h"

namespace CE
{

    FReorderableStack::FReorderableStack()
    {
        m_BackgroundShape = FRectangle();
        m_BorderStyle = FPenStyle::SolidLine;
    }

    void FReorderableStack::Construct()
    {
        Super::Construct();
        
    }

    void FReorderableStack::UpdateOrder()
    {

    }

    void FReorderableStack::CalculateIntrinsicSize()
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

        for (WeakRef<FWidget> childWeak : children)
        {
            Ref<FWidget> child = childWeak.Lock();
            if (!child)
                continue;
            if (!child->Enabled())
                continue;

            child->CalculateIntrinsicSize();

            Vec2 childSize = child->GetIntrinsicSize();
            Vec4 childMargin = child->Margin();

            if (m_Direction == FStackBoxDirection::Horizontal)
            {
                contentSize.width += childSize.width + childMargin.left + childMargin.right;
                contentSize.height = Math::Max(contentSize.height, childSize.height + childMargin.top + childMargin.bottom);
            }
            else if (m_Direction == FStackBoxDirection::Vertical)
            {
                contentSize.height += childSize.height + childMargin.top + childMargin.bottom;
                contentSize.width = Math::Max(contentSize.width, childSize.width + childMargin.left + childMargin.right);
            }
        }

        if (m_Direction == FStackBoxDirection::Horizontal)
        {
            contentSize.width += m_Gap * (children.GetSize() - 1);
        }
        else if (m_Direction == FStackBoxDirection::Vertical)
        {
            contentSize.height += m_Gap * (children.GetSize() - 1);
        }

        intrinsicSize.width += contentSize.width;
        intrinsicSize.height += contentSize.height;

        intrinsicSize.width = Math::Clamp(intrinsicSize.width,
            m_MinWidth + m_Padding.left + m_Padding.right,
            m_MaxWidth + m_Padding.left + m_Padding.right);

        intrinsicSize.height = Math::Clamp(intrinsicSize.height,
            m_MinHeight + m_Padding.top + m_Padding.bottom,
            m_MaxHeight + m_Padding.top + m_Padding.bottom);
    }

    void FReorderableStack::PlaceSubWidgets()
    {

    }

    void FReorderableStack::OnPaint(FPainter* painter)
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

        if ((m_BackgroundShape.GetShapeType() != FShapeType::None && m_Background.GetBrushStyle() != FBrushStyle::None) ||
            (m_BorderWidth > 0 && m_BorderColor.a > 0))
        {
            painter->SetBrush(m_Background);

            if (m_BorderWidth > 0 && m_BorderColor.a > 0)
            {
                painter->SetPen(FPen(m_BorderColor, m_BorderWidth, m_BorderStyle));
            }
            else
            {
                painter->SetPen(FPen());
            }

            switch (m_Background.GetBrushStyle())
            {
            case FBrushStyle::None:
                break;
            case FBrushStyle::SolidFill:
                break;
            case FBrushStyle::Image:
                break;
            case FBrushStyle::Gradient:
                break;
            }

            isCulled = !painter->DrawShape(Rect::FromSize(Vec2(), computedSize), m_BackgroundShape);
        }

        if (activeItem && activeItem->Enabled() && activeItem->Visible())
        {
            if (activeItem->IsTranslationOnly())
            {
                painter->PushChildCoordinateSpace(activeItem->GetComputedPosition() + activeItem->Translation());
            }
            else
            {
                painter->PushChildCoordinateSpace(activeItem->GetLocalTransform());
            }

            activeItem->OnPaint(painter);

            painter->PopChildCoordinateSpace();
        }

        for (const auto& child : children)
        {
            if (!child->Enabled() || !child->Visible())
                continue;
            if (child == activeItem)
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

    FWidget* FReorderableStack::HitTest(Vec2 localMousePos)
    {
        FWidget* thisHitTest = SelfHitTest(localMousePos);
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

        if (activeItem && activeItem->Enabled())
        {
            if (FWidget* result = activeItem->HitTest(transformedMousePos))
            {
                return result;
            }
        }

        for (int i = children.GetSize() - 1; i >= 0; --i)
        {
            Ref<FWidget> child = children[i].Get();
            if (!child->Enabled() || child == activeItem)
                continue;

            if (FWidget* result = child->HitTest(transformedMousePos))
            {
                return result;
            }
        }

        return thisHitTest;
    }
}

