#include "FusionCore.h"

namespace CE
{
	constexpr f32 SplitterTolerance = 2;

    FSplitBox::FSplitBox()
    {
		m_SplitterDrawRatio = 1.0f;
		m_SplitterSize = 5.0f;
		m_SplitterHoverBackground = Color::RGBA(255, 255, 255, 120);
		m_SplitterBackground = Colors::Clear;
		m_Direction = FSplitDirection::Horizontal;
    }

    void FSplitBox::CalculateIntrinsicSize()
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

			if (m_Direction == FSplitDirection::Horizontal)
			{
				contentSize.width += childSize.width;
				contentSize.height = Math::Max(contentSize.height, childSize.height + childMargin.top + childMargin.bottom);
			}
			else if (m_Direction == FSplitDirection::Vertical)
			{
				contentSize.height += childSize.height;
				contentSize.width = Math::Max(contentSize.width, childSize.width + childMargin.left + childMargin.right);
			}
		}

		if (m_Direction == FSplitDirection::Horizontal)
		{
			contentSize.width += m_SplitterSize * (children.GetSize() - 1);
		}
		else if (m_Direction == FSplitDirection::Vertical)
		{
			contentSize.height += m_SplitterSize * (children.GetSize() - 1);
		}

		intrinsicSize.width += contentSize.width;
		intrinsicSize.height += contentSize.height;

		ApplyIntrinsicSizeConstraints();
    }

    void FSplitBox::PlaceSubWidgets()
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

		if (m_Direction == FSplitDirection::Horizontal)
		{
			crossAxisSize = availableSize.height;
			remainingSize = availableSize.width;
		}
		else if (m_Direction == FSplitDirection::Vertical)
		{
			crossAxisSize = availableSize.width;
			remainingSize = availableSize.height;
		}

		remainingSize -= m_SplitterSize * (children.GetSize() - 1);

		for (const auto& child : children)
		{
			if (!child->Enabled())
				continue;

			Vec2 childIntrinsicSize = child->GetIntrinsicSize();

			child->SetComputedPosition(curPos + Vec2(child->Margin().left, child->Margin().top));

			CE::VAlign vertAlign = child->VAlign();
			if (vertAlign == CE::VAlign::Auto)
				vertAlign = m_ContentVAlign;

			CE::HAlign horiAlign = child->HAlign();
			if (horiAlign == HAlign::Auto)
				horiAlign = m_ContentHAlign;

			f32 fillRatio = child->FillRatio();

			if (m_Direction == FSplitDirection::Horizontal)
			{
				f32 minWidth = (child->MinWidth() + child->Padding().left + child->Padding().right);
				fillRatio = Math::Max(fillRatio, minWidth / remainingSize);

				child->computedSize.width = remainingSize * fillRatio;
				child->computedSize.height = availableSize.height;

				curPos.x += child->computedSize.width + m_SplitterSize;
			}
			else if (m_Direction == FSplitDirection::Vertical)
			{
				f32 minHeight = (child->MinHeight() + child->Padding().top + child->Padding().bottom);
				fillRatio = Math::Max(fillRatio, minHeight / remainingSize);

				child->computedSize.height = remainingSize * fillRatio;
				child->computedSize.width = availableSize.width;

				curPos.y += child->computedSize.height + m_SplitterSize;
			}

			child->PlaceSubWidgets();
		}
    }

    void FSplitBox::Construct()
    {
        Super::Construct();
        
    }

    void FSplitBox::OnPaint(FPainter* painter)
    {
		Super::OnPaint(painter);

		if (children.IsEmpty() || !Enabled())
			return;

		
		if (m_ClipChildren)
		{
			painter->PushClipRect(Matrix4x4::Identity(), computedSize);
		}

		Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right,
			m_Padding.top + m_Padding.bottom);

		if ((hoveredSplitIndex >= 0 && isCursorPushed) || m_SplitterBackground.a > 0.01f)
		{
			
		}

		if (m_SplitterBackground.a > 0.01f)
		{
			for (int i = 0; i < children.GetSize() - 1; ++i)
			{
				f32 offset = (1.0f - m_SplitterDrawRatio) * m_SplitterSize * 0.5f;

				FWidget* left = children[i].Get();
				Vec2 splitterPos = left->GetComputedPosition() +
					(m_Direction == FSplitDirection::Horizontal
						? Vec2(left->computedSize.width + offset, 0)
						: Vec2(0, left->computedSize.height + offset));
				Vec2 splitterSize = m_Direction == FSplitDirection::Horizontal
					? Vec2(m_SplitterSize * m_SplitterDrawRatio, availableSize.y)
					: Vec2(availableSize.x, m_SplitterSize * m_SplitterDrawRatio);

				painter->SetBrush(m_SplitterBackground);
				painter->SetPen(FPen());
				painter->DrawRect(Rect::FromSize(splitterPos, splitterSize));
			}
		}

		if (hoveredSplitIndex >= 0 && isCursorPushed)
		{
			f32 offset = (1.0f - m_SplitterDrawRatio) * m_SplitterSize * 0.5f;

			FWidget* left = children[hoveredSplitIndex].Get();
			Vec2 splitterPos = left->GetComputedPosition() + 
				(m_Direction == FSplitDirection::Horizontal 
					? Vec2(left->computedSize.width + offset, 0)
					: Vec2(0, left->computedSize.height + offset));
			Vec2 splitterSize = m_Direction == FSplitDirection::Horizontal
				? Vec2(m_SplitterSize * m_SplitterDrawRatio, availableSize.y)
				: Vec2(availableSize.x, m_SplitterSize * m_SplitterDrawRatio);

			if (m_SplitterHoverBackground.a > 0)
			{
				painter->SetBrush(m_SplitterHoverBackground);
				painter->SetPen(FPen());
				painter->DrawRect(Rect::FromSize(splitterPos, splitterSize));
			}
		}

		if ((hoveredSplitIndex >= 0 && isCursorPushed) || m_SplitterBackground.a > 0.01f)
		{
			
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

    void FSplitBox::HandleEvent(FEvent* event)
    {
		if (children.GetSize() <= 1)
		{
			Super::HandleEvent(event);
			return;
		}

		auto app = FusionApplication::Get();

		if (event->IsMouseEvent())
		{
			FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

			Vec2 localMousePos = mouseEvent->mousePosition;
			localMousePos = globalTransform.GetInverse() * Vec4(localMousePos.x, localMousePos.y, 0, 1);

			if (draggedSplitIndex == -1 && 
				(mouseEvent->type == FEventType::MouseMove || mouseEvent->type == FEventType::MouseEnter || mouseEvent->type == FEventType::MouseLeave))
			{
				bool isInside = false;

				for (int i = 0; mouseEvent->type != FEventType::MouseLeave && i < children.GetSize() - 1; ++i)
				{
					FWidget* child = children[i].Get();
					FWidget* nextChild = children[i + 1].Get();

					if (m_Direction == FSplitDirection::Horizontal)
					{
						if (localMousePos.x >= child->computedPosition.x + child->computedSize.x &&
							localMousePos.x <= nextChild->computedPosition.x)
						{
							isInside = true;
							if (!isCursorPushed)
							{
								hoveredSplitIndex = i;
								MarkDirty();
								app->PushCursor(SystemCursor::SizeHorizontal);
								isCursorPushed = true;
							}
							break;
						}
					}
					else if (m_Direction == FSplitDirection::Vertical)
					{
						if (localMousePos.y >= child->computedPosition.y + child->computedSize.y &&
							localMousePos.y <= nextChild->computedPosition.y)
						{
							isInside = true;
							if (!isCursorPushed)
							{
								hoveredSplitIndex = i;
								MarkDirty();
								app->PushCursor(SystemCursor::SizeVertical);
								isCursorPushed = true;
							}
							break;
						}
					}
				}

				if (!isInside && isCursorPushed)
				{
					hoveredSplitIndex = -1;
					MarkDirty();
					isCursorPushed = false;
					app->PopCursor();
				}
			}

			if (mouseEvent->type == FEventType::DragBegin && mouseEvent->sender == this)
			{
				FDragEvent* dragEvent = static_cast<FDragEvent*>(mouseEvent);
				dragStartMousePos = localMousePos;

				for (int i = 0; i < children.GetSize() - 1; ++i)
				{
					FWidget* child = children[i].Get();
					FWidget* nextChild = children[i + 1].Get();

					leftFillRatio = child->FillRatio();
					rightFillRatio = nextChild->FillRatio();

					if (m_Direction == FSplitDirection::Horizontal)
					{
						if (localMousePos.x >= child->computedPosition.x + child->computedSize.x &&
							localMousePos.x <= nextChild->computedPosition.x)
						{
							draggedSplitIndex = hoveredSplitIndex  = i;
							dragEvent->draggedWidget = this;
							dragEvent->Consume(this);
							break;
						}
					}
					else if (m_Direction == FSplitDirection::Vertical)
					{
						if (localMousePos.y >= child->computedPosition.y + child->computedSize.y &&
							localMousePos.y <= nextChild->computedPosition.y)
						{
							draggedSplitIndex = hoveredSplitIndex = i;
							dragEvent->draggedWidget = this;
							dragEvent->Consume(this);
							break;
						}
					}
				}
			}
			else if (mouseEvent->type == FEventType::DragMove)
			{
				FDragEvent* dragEvent = static_cast<FDragEvent*>(mouseEvent);

				float mouseDelta = m_Direction == FSplitDirection::Horizontal
					? localMousePos.x - dragStartMousePos.x
					: localMousePos.y - dragStartMousePos.y;
				float totalSize = m_Direction == FSplitDirection::Horizontal
					? (computedSize.width - m_Padding.left - m_Padding.right)
					: (computedSize.height - m_Padding.top - m_Padding.bottom);
				totalSize -= m_SplitterSize * children.GetSize();

				if (draggedSplitIndex >= 0 && draggedSplitIndex < children.GetSize() - 1)
				{
					FWidget* left = children[draggedSplitIndex].Get();
					FWidget* right = children[draggedSplitIndex + 1].Get();

					f32 leftRatio = leftFillRatio + mouseDelta / totalSize;
					f32 rightRatio = rightFillRatio - mouseDelta / totalSize;

					if (leftRatio > 0.0001f && rightRatio > 0.0001f)
					{
						left->FillRatio(leftRatio);
						right->FillRatio(rightRatio);
					}

					dragEvent->draggedWidget = this;
					dragEvent->Consume(this);

					m_OnSplitterDragged(this);
				}
				else
				{
					draggedSplitIndex = -1;
				}
			}
			else if (mouseEvent->type == FEventType::DragEnd)
			{
				FDragEvent* dragEvent = static_cast<FDragEvent*>(mouseEvent);

				draggedSplitIndex = hoveredSplitIndex = -1;

				if (isCursorPushed)
				{
					MarkDirty();
					app->PopCursor();
					isCursorPushed = false;
				}

				dragEvent->Consume(this);
			}
		}

	    Super::HandleEvent(event);
    }

}

