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

		if (m_Direction == FStackBoxDirection::Horizontal)
		{
			availableSize.width -= m_Gap * (children.GetSize() - 1);

			crossAxisSize = availableSize.height;
			remainingSize = availableSize.width;
		}
		else if (m_Direction == FStackBoxDirection::Vertical)
		{
			availableSize.height -= m_Gap * (children.GetSize() - 1);

			crossAxisSize = availableSize.width;
			remainingSize = availableSize.height;
		}

		f32 totalFillRatio = 0.0f;

		for (const auto& child : children)
		{
			if (!child->Enabled())
				continue;

			Vec2 childIntrinsicSize = child->GetIntrinsicSize();

			if (m_Direction == FStackBoxDirection::Horizontal)
			{
				if (child->m_FillRatio > 0)
				{
					totalFillRatio += child->m_FillRatio;
					remainingSize -= child->Margin().left + child->Margin().right +
						child->Padding().left + child->Padding().right + child->MinWidth();
				}
				else
				{
					remainingSize -= childIntrinsicSize.width + child->Margin().left + child->Margin().right;
				}
			}
			else if (m_Direction == FStackBoxDirection::Vertical)
			{
				if (child->m_FillRatio > 0)
				{
					totalFillRatio += child->m_FillRatio;
					remainingSize -= child->Margin().top + child->Margin().bottom +
						child->Padding().top + child->Padding().bottom + child->MinHeight();
				}
				else
				{
					remainingSize -= childIntrinsicSize.height + child->Margin().top + child->Margin().bottom;
				}
			}
		}

		f32 contentAlignmentRatio = 0.0f;

		if (m_Direction == FStackBoxDirection::Horizontal)
		{
			switch (m_ContentHAlign)
			{
			case HAlign::Auto:
				break;
			case HAlign::Fill:
				break;
			case HAlign::Left:
				contentAlignmentRatio = 0.0f;
				break;
			case HAlign::Center:
				contentAlignmentRatio = 0.5f;
				break;
			case HAlign::Right:
				contentAlignmentRatio = 1.0f;
				break;
			}

			curPos.x += contentAlignmentRatio * remainingSize;
		}
		else if (m_Direction == FStackBoxDirection::Vertical)
		{
			switch (m_ContentVAlign)
			{
			case VAlign::Auto:
				break;
			case VAlign::Fill:
				break;
			case VAlign::Top:
				contentAlignmentRatio = 0.0f;
				break;
			case VAlign::Center:
				contentAlignmentRatio = 0.5f;
				break;
			case VAlign::Bottom:
				contentAlignmentRatio = 1.0f;
				break;
			}

			curPos.y += contentAlignmentRatio * remainingSize;
		}

		for (int i = 0; i < children.GetSize(); i++)
		{
			Ref<FWidget> child = children[i].Lock();
			if (!child)
				continue;
			if (!child->Enabled())
				continue;

			Vec2 childIntrinsicSize = child->GetIntrinsicSize();

			child->computedPosition = curPos + Vec2(child->m_Margin.left, child->m_Margin.top);

			CE::VAlign vertAlign = child->m_VAlign;
			if (vertAlign == CE::VAlign::Auto)
				vertAlign = m_ContentVAlign;

			CE::HAlign horiAlign = child->m_HAlign;
			if (horiAlign == HAlign::Auto)
				horiAlign = m_ContentHAlign;

			if (m_Direction == FStackBoxDirection::Horizontal)
			{
				switch (vertAlign)
				{
				case VAlign::Auto:
				case VAlign::Fill:
					child->computedSize.height = availableSize.height;
					break;
				case VAlign::Top:
					child->computedSize.height = Math::Min(childIntrinsicSize.height, availableSize.height);
					break;
				case VAlign::Center:
					child->computedSize.height = Math::Min(childIntrinsicSize.height, availableSize.height);
					//child->computedPosition.y = child->computedPosition.y + (availableSize.height - childIntrinsicSize.height) * 0.5f;
					child->computedPosition.y = child->computedPosition.y + (availableSize.height - child->computedSize.height) * 0.5f;
					break;
				case VAlign::Bottom:
					child->computedSize.height = Math::Min(childIntrinsicSize.height, availableSize.height);
					//child->computedPosition.y = child->computedPosition.y + (availableSize.height - childIntrinsicSize.height);
					child->computedPosition.y = child->computedPosition.y + (availableSize.height - child->computedSize.height);
					break;
				}

				if (child->m_FillRatio > 0)
				{
					child->computedSize.width = child->Margin().left + child->Margin().right +
						child->Padding().left + child->Padding().right + child->MinWidth() +
						remainingSize * child->m_FillRatio / totalFillRatio;
				}
				else
				{
					child->computedSize.width = childIntrinsicSize.width;
				}

				child->ApplySizeConstraints();

				child->PlaceSubWidgets();

				curPos.x += child->computedSize.width + child->m_Margin.left + child->m_Margin.right + m_Gap;
			}
			else if (m_Direction == FStackBoxDirection::Vertical)
			{
				switch (horiAlign)
				{
				case HAlign::Auto:
				case HAlign::Fill:
					child->computedSize.width = availableSize.width;
					break;
				case HAlign::Left:
					child->computedSize.width = Math::Min(childIntrinsicSize.width, availableSize.width);
					break;
				case HAlign::Center:
					child->computedSize.width = Math::Min(childIntrinsicSize.width, availableSize.width);
					child->computedPosition.x = curPos.x + child->m_Margin.left + (availableSize.width - child->computedSize.width) * 0.5f;
					break;
				case HAlign::Right:
					child->computedSize.width = Math::Min(childIntrinsicSize.width, availableSize.width);
					child->computedPosition.x = curPos.x + child->m_Margin.left + (availableSize.width - child->computedSize.width);
					break;
				}

				if (child->m_FillRatio > 0)
				{
					child->computedSize.height = child->Margin().top + child->Margin().bottom +
						child->Padding().top + child->Padding().bottom + child->MinHeight() +
						remainingSize * child->m_FillRatio / totalFillRatio;
				}
				else
				{
					child->computedSize.height = childIntrinsicSize.height;
				}

				child->ApplySizeConstraints();

				child->PlaceSubWidgets();

				curPos.y += child->computedSize.height + child->m_Margin.top + child->m_Margin.bottom + m_Gap;
			}
		}

		OnPostComputeLayout();
    }

    void FReorderableStack::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);

        if (children.IsEmpty() || !Enabled())
            return;

		if (activeItem && !children.Exists(activeItem))
		{
			activeItem = nullptr;
		}

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

    void FReorderableStack::SetActiveItem(Ref<FReorderableStackItem> item)
    {
		activeItem = item;
    }

    void FReorderableStack::OnActiveItemDragged(FDragEvent* dragEvent)
    {
		
    }
}

