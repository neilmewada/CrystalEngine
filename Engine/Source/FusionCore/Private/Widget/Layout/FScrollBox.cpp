#include "FusionCore.h"

namespace CE
{
    constexpr f32 MinScrollBarHeight = 12.5f;

    FScrollBox::FScrollBox()
    {
        m_VerticalScroll = true;
        m_HorizontalScroll = false;
        m_ClipChildren = true;

        m_ScrollBarWidth = 8;
        m_ScrollBarShape = FRoundedRectangle(4);
        m_ScrollBarMargin = 2.5f;

        m_VerticalScrollSensitivity = m_HorizontalScrollSensitivity = 15.0f;
    }

    void FScrollBox::CalculateIntrinsicSize()
    {
        intrinsicSize = Vec2(m_MinWidth + m_Padding.left + m_Padding.right,
            m_MinHeight + m_Padding.top + m_Padding.bottom);

        FWidget* child = GetChild().Get();

        if (!child || !child->Enabled())
            return;

        child->CalculateIntrinsicSize();

        Vec2 childSize = child->GetIntrinsicSize();
        const Vec4& childMargin = child->Margin();

        intrinsicSize.width = Math::Max(intrinsicSize.width, childSize.width + m_Padding.left + m_Padding.right + childMargin.left + childMargin.right);
        intrinsicSize.height = Math::Max(intrinsicSize.height, childSize.height + m_Padding.top + m_Padding.bottom + childMargin.top + childMargin.bottom);

        ApplyIntrinsicSizeConstraints();
    }

    void FScrollBox::PlaceSubWidgets()
    {
        ApplySizeConstraints();
        UpdateLocalTransform();

        FWidget* child = GetChild().Get();

        if (!child || !child->Enabled())
            return;

        Vec4 childMargin = child->Margin();
        Vec2 childIntrinsicSize = child->GetIntrinsicSize();
        Vec2 availableSize = computedSize - Vec2(m_Padding.left + m_Padding.right + childMargin.left + childMargin.right,
            m_Padding.top + m_Padding.bottom + childMargin.top + childMargin.bottom);

        if (isVerticalScrollVisible && !m_HideVerticalScroll)
        {
            availableSize.x -= m_ScrollBarMargin * 2 + m_ScrollBarWidth;
        }
        if (isHorizontalScrollVisible && !m_HideHorizontalScroll)
        {
            availableSize.y -= m_ScrollBarMargin * 2 + m_ScrollBarWidth;
        }

        child->computedPosition = Vec2(m_Padding.left + childMargin.left, m_Padding.top + childMargin.top);
        child->computedSize = computedSize -
            Vec2(m_Padding.left + m_Padding.right + childMargin.left + childMargin.right,
                m_Padding.top + m_Padding.bottom + childMargin.top + childMargin.bottom);

        CE::VAlign childVAlign = child->VAlign();
        CE::HAlign childHAlign = child->HAlign();

        if (VerticalScroll() && childVAlign == VAlign::Auto)
        {
            childVAlign = VAlign::Top;
        }
        if (HorizontalScroll() && childHAlign == HAlign::Auto)
        {
            childHAlign = HAlign::Left;
        }

        if ((VerticalScroll() && childVAlign == VAlign::Fill) || (HorizontalScroll() && childHAlign == HAlign::Fill))
        {
            CE_LOG(Warn, All, "FScrollBox's child widget has a VAlign and/or HAlign set to Fill!\n"
                              "This will prevent the scrolling behaviour from working properly.");
        }

        switch (childVAlign)
        {
        case VAlign::Auto:
        case VAlign::Fill:
            break;
        case VAlign::Top:
            child->computedSize.height = childIntrinsicSize.height;
            child->computedPosition.y = m_Padding.top + childMargin.top;
            break;
        case VAlign::Center:
            child->computedSize.height = childIntrinsicSize.height;
            child->computedPosition.y = m_Padding.top + childMargin.top + (availableSize.height - childIntrinsicSize.height) * 0.5f;
            break;
        case VAlign::Bottom:
            child->computedSize.height = childIntrinsicSize.height;
            child->computedPosition.y = m_Padding.top + childMargin.top + (availableSize.height - childIntrinsicSize.height);
            break;
        }
        
        switch (childHAlign)
        {
        case HAlign::Auto:
        case HAlign::Fill:
            if (isVerticalScrollVisible && !m_HideVerticalScroll)
            {
                child->computedSize.width -= m_ScrollBarMargin * 2 + m_ScrollBarWidth;
            }
            break;
        case HAlign::Left:
            child->computedSize.width = childIntrinsicSize.width;
            child->computedPosition.x = m_Padding.left + childMargin.left;
            break;
        case HAlign::Center:
            child->computedSize.width = childIntrinsicSize.width;
            child->computedPosition.x = m_Padding.left + childMargin.left + (availableSize.width - childIntrinsicSize.width) * 0.5f;
            break;
        case HAlign::Right:
            child->computedSize.width = childIntrinsicSize.width;
            child->computedPosition.x = m_Padding.left + childMargin.left + (availableSize.width - childIntrinsicSize.width);
            break;
        }

        child->PlaceSubWidgets();
        
        if (child->computedSize.height > availableSize.height && VerticalScroll())
        {
	        if (!isVerticalScrollVisible)
	        {
                isVerticalScrollVisible = true;
                MarkLayoutDirty();
	        }
        }
        else if (isVerticalScrollVisible)
        {
            isVerticalScrollVisible = false;
            MarkLayoutDirty();
        }

        if (child->computedSize.width > availableSize.width && HorizontalScroll())
        {
	        if (!isHorizontalScrollVisible)
	        {
                isHorizontalScrollVisible = true;
                MarkLayoutDirty();
	        }
        }
        else if (isHorizontalScrollVisible)
        {
            isHorizontalScrollVisible = false;
            MarkLayoutDirty();
        }

        OnPostComputeLayout();
    }

    void FScrollBox::OnPostComputeLayout()
    {
	    Super::OnPostComputeLayout();

        if (VerticalScroll())
	    {
		    NormalizedScrollY(Math::Clamp01(NormalizedScrollY()));
	    }
        if (HorizontalScroll())
        {
	        NormalizedScrollX(Math::Clamp01(NormalizedScrollX()));
        }
    }

    Rect FScrollBox::GetVerticalScrollBarRect()
    {
        FWidget* child = GetChild().Get();
        if (!child)
            return Rect();

        f32 scrollBarHeight = computedSize.y * (computedSize.y / child->computedSize.y);
        scrollBarHeight = Math::Max(scrollBarHeight, MinScrollBarHeight);

        f32 normalizedScrollY = NormalizedScrollY();
        Vec2 barPos = Vec2(computedPosition.x + computedSize.x - m_ScrollBarMargin - m_ScrollBarWidth,
            computedPosition.y + normalizedScrollY * (computedSize.y - scrollBarHeight));

        return Rect::FromSize(barPos, Vec2(m_ScrollBarWidth, scrollBarHeight));
    }

    Rect FScrollBox::GetHorizontalScrollBarRect()
    {
        FWidget* child = GetChild().Get();
        if (!child)
            return Rect();

        f32 endOffset = 0;
        if (isVerticalScrollVisible && !m_HideVerticalScroll)
        {
            endOffset = m_ScrollBarMargin * 2 + m_ScrollBarWidth;
        }

        f32 computedSizeX = computedSize.x - endOffset;

        f32 scrollBarSize = computedSizeX * computedSizeX / child->computedSize.x;
        scrollBarSize = Math::Max(scrollBarSize, MinScrollBarHeight);

        f32 normalizedScrollX = NormalizedScrollX();
        Vec2 barPos = Vec2(computedPosition.x + normalizedScrollX * (computedSizeX - scrollBarSize),
            computedPosition.y + computedSize.y - m_ScrollBarMargin - m_ScrollBarWidth);

        return Rect::FromSize(barPos, Vec2(scrollBarSize, m_ScrollBarWidth));
    }

    void FScrollBox::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        if (!GetChild())
            return;

        if (isVerticalScrollVisible && !m_HideVerticalScroll)
        {
            if (m_ScrollBarBackground.IsValidBrush() || m_ScrollBarBackgroundPen.IsValidPen())
            {
                painter->SetBrush(m_ScrollBarBackground);
                painter->SetPen(m_ScrollBarBackgroundPen);

                Vec2 pos = Vec2(computedSize.x - m_ScrollBarMargin * 2 - m_ScrollBarWidth, 0);
                Vec2 size = Vec2(m_ScrollBarMargin * 2 + m_ScrollBarWidth, computedSize.y);

                painter->DrawRect(Rect::FromSize(pos, size));
            }
            
            if (isVerticalScrollHighlighted)
            {
                painter->SetBrush(m_ScrollBarHoverBrush);
                painter->SetPen(m_ScrollBarHoverPen);
            }
            else
            {
                painter->SetBrush(m_ScrollBarBrush);
                painter->SetPen(m_ScrollBarPen);
            }

            painter->DrawShape(GetVerticalScrollBarRect().Translate(-computedPosition), m_ScrollBarShape);
        }

        if (isHorizontalScrollVisible && !m_HideHorizontalScroll)
        {
            if (m_ScrollBarBackground.IsValidBrush() || m_ScrollBarBackgroundPen.IsValidPen())
            {
                painter->SetBrush(m_ScrollBarBackground);
                painter->SetPen(m_ScrollBarBackgroundPen);

                f32 horiOffset = 0;
                if (isVerticalScrollVisible)
                    horiOffset = m_ScrollBarMargin * 2 + m_ScrollBarWidth;

                Vec2 pos = Vec2(0, computedSize.y - m_ScrollBarMargin * 2 - m_ScrollBarWidth);
                Vec2 size = Vec2(computedSize.x - horiOffset, m_ScrollBarMargin * 2 + m_ScrollBarWidth);

                painter->DrawRect(Rect::FromSize(pos, size));
            }

            if (isHorizontalScrollHighlighted)
            {
                painter->SetBrush(m_ScrollBarHoverBrush);
                painter->SetPen(m_ScrollBarHoverPen);
            }
            else
            {
                painter->SetBrush(m_ScrollBarBrush);
                painter->SetPen(m_ScrollBarPen);
            }

            painter->DrawShape(GetHorizontalScrollBarRect().Translate(-computedPosition), m_ScrollBarShape);
        }
    }

    void FScrollBox::HandleEvent(FEvent* event)
    {
        FWidget* child = GetChild().Get();

        if (event->IsMouseEvent() && child)
        {
            FMouseEvent* mouseEvent = (FMouseEvent*)event;
            Vec2 localMousePos = mouseEvent->mousePosition;
            localMousePos = globalTransform.GetInverse() * Vec4(localMousePos.x, localMousePos.y, 0, 1);

            bool isVScroll = false;
            bool isHScroll = false;

            if (isVerticalScrollVisible && !m_HideVerticalScroll)
            {
                // localMousePos already contains this widget's computed position in it.
                Rect scrollBar = GetVerticalScrollBarRect().Translate(-computedPosition);

                if (scrollBar.Contains(localMousePos))
                {
                    isVScroll = true;
                }
            }

            if (isHorizontalScrollVisible && !m_HideHorizontalScroll)
            {
                // localMousePos already contains this widget's computed position in it.
                Rect scrollBar = GetHorizontalScrollBarRect().Translate(-computedPosition);

                if (scrollBar.Contains(localMousePos))
                {
                    isHScroll = true;
                }
            }

            if (mouseEvent->type == FEventType::MousePress && event->sender == this && !isVScroll && !isHScroll)
            {
                m_OnBackgroundClicked(this);
            }
            
            if (mouseEvent->type == FEventType::DragBegin && event->sender == this)
            {
                FDragEvent* dragEvent = static_cast<FDragEvent*>(mouseEvent);

                if (isVScroll)
                {
                    dragEvent->draggedWidget = this;
                    event->Consume(this);

                    isVerticalScrollDragged = true;
                }
                else if (isHScroll)
                {
                    dragEvent->draggedWidget = this;
                    event->Consume(this);

                    isHorizontalScrollDragged = true;
                }
            }

            if (mouseEvent->type == FEventType::MouseEnter || mouseEvent->type == FEventType::MouseMove || mouseEvent->type == FEventType::MouseLeave)
            {
                if (isVerticalScrollHighlighted != (isVScroll || isVerticalScrollDragged))
                {
                    isVerticalScrollHighlighted = isVScroll || isVerticalScrollDragged;
                    MarkDirty();
                }

                if (isHorizontalScrollHighlighted != (isHScroll || isHorizontalScrollDragged))
                {
                    isHorizontalScrollHighlighted = isHScroll || isHorizontalScrollDragged;
                    MarkDirty();
                }
            }

            if (mouseEvent->type == FEventType::MouseWheel)
            {
                Vec2 wheelDelta = mouseEvent->wheelDelta;

                // Reverse mouse wheel behavior on Mac with SDL
                // TODO: Test this on Windows & Linux and make sure it works
                if (isHorizontalScrollVisible && EnumHasFlag(mouseEvent->keyModifiers, KeyModifier::Shift) &&
                    PlatformMisc::GetCurrentPlatform() != PlatformName::Mac)
                {
                    wheelDelta.x = wheelDelta.y;
                    wheelDelta.y = 0;
                }

	            if (isVerticalScrollVisible)
	            {
                    f32 originalHeight = computedSize.height;
                    f32 contentHeight = child->computedSize.height;

                    f32 normalizedScrollY = NormalizedScrollY();
                    normalizedScrollY += -wheelDelta.y * m_VerticalScrollSensitivity / (contentHeight - originalHeight) * GetContext()->GetScaling();
                    NormalizedScrollY(normalizedScrollY);
	            }

                if (isHorizontalScrollVisible)
                {
                    f32 originalWidth = computedSize.width;
                    f32 contentWidth = child->computedSize.width;

                    if (isVerticalScrollVisible)
                        originalWidth -= m_ScrollBarMargin * 2 + m_ScrollBarWidth;

                    f32 normalizedScrollX = NormalizedScrollX();
                    normalizedScrollX += -wheelDelta.x * m_HorizontalScrollSensitivity / (contentWidth - originalWidth) * GetContext()->GetScaling();
                    NormalizedScrollX(normalizedScrollX);
                }
            }
        }

        if (event->IsDragEvent())
        {
            FDragEvent* dragEvent = static_cast<FDragEvent*>(event);
            Vec2 mouseDelta = (dragEvent->mousePosition - dragEvent->prevMousePosition) * FusionApplication::Get()->GetDefaultScalingFactor();

            if (dragEvent->type == FEventType::DragMove)
            {
                if (isVerticalScrollDragged)
                {
                    f32 scrollBarHeight = GetVerticalScrollBarRect().GetSize().height;
                    f32 normalizedScrollY = NormalizedScrollY();

                    f32 originalHeight = computedSize.y;

                    normalizedScrollY += mouseDelta.y / (originalHeight - scrollBarHeight);
                    normalizedScrollY = Math::Clamp01(normalizedScrollY);

                    NormalizedScrollY(normalizedScrollY);

                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);
                }
                else if (isHorizontalScrollDragged)
                {
                    f32 scrollBarLength = GetHorizontalScrollBarRect().GetSize().width;
                    f32 normalizedScrollX = NormalizedScrollX();

                    f32 originalWidth = computedSize.x;
                    if (isVerticalScrollVisible)
                    {
                        originalWidth -= m_ScrollBarMargin * 2 + m_ScrollBarWidth;
                    }

                    normalizedScrollX += mouseDelta.x / (originalWidth - scrollBarLength);
                    normalizedScrollX = Math::Clamp01(normalizedScrollX);

                    NormalizedScrollX(normalizedScrollX);

                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);
                }
            }
            else if (dragEvent->type == FEventType::DragEnd)
            {
                if (isVerticalScrollDragged)
                {
                    isVerticalScrollDragged = false;
                    isVerticalScrollHighlighted = false;

                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    MarkDirty();
                }
                else if (isHorizontalScrollDragged)
                {
                    isHorizontalScrollDragged = false;
                    isHorizontalScrollHighlighted = false;

                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);

                    MarkDirty();
                }
            }
        }

	    Super::HandleEvent(event);
    }

    FWidget* FScrollBox::HitTest(Vec2 mousePosition)
    {
        FWidget* hitResult = Super::HitTest(mousePosition);
        if (hitResult == nullptr)
            return nullptr;

        FWidget* child = GetChild().Get();
        if (isVerticalScrollDragged || isHorizontalScrollDragged)
        {
            return this;
        }

        auto invScale = Vec3(1 / m_Scale.x, 1 / m_Scale.y, 1.0f);

        Vec2 transformedMousePos;

    	if (child != nullptr && ((isVerticalScrollVisible && !m_HideVerticalScroll) || (isHorizontalScrollVisible && !m_HideHorizontalScroll)))
    	{
            transformedMousePos = (Matrix4x4::Translation(computedSize * m_Anchor) *
                Matrix4x4::Angle(-m_Angle) *
                Matrix4x4::Scale(invScale) *
                Matrix4x4::Translation(-computedPosition - m_Translation - computedSize * m_Anchor)) *
                Vec4(mousePosition.x, mousePosition.y, 0, 1);
    	}

        if (child && isVerticalScrollVisible && !m_HideVerticalScroll)
        {
            Vec2 barSize = Vec2(m_ScrollBarWidth, computedSize.y);
            Rect barRegion = Rect::FromSize(Vec2(computedSize.x - m_ScrollBarMargin - m_ScrollBarWidth, 
                0), barSize);

            if (barRegion.Contains(transformedMousePos))
            {
                return this;
            }
        }

        if (child && isHorizontalScrollVisible && !m_HideHorizontalScroll)
        {
            Rect barRegion = Rect(computedPosition.x, computedPosition.y + computedSize.y - m_ScrollBarMargin * 2 - m_ScrollBarWidth, 
                computedPosition.x + computedSize.x, computedPosition.y + computedSize.y);

            if (barRegion.Contains(transformedMousePos))
            {
                return this;
            }
        }

        return hitResult;
    }

    FScrollBox& FScrollBox::NormalizedScrollY(f32 value)
    {
        value = Math::Clamp01(value);

        FWidget* child = GetChild().Get();
        f32 scrollBarSection = m_ScrollBarMargin * 2 + m_ScrollBarWidth;

        if (child)
        {
            Vec2 translation = child->Translation();
            if (isHorizontalScrollVisible)
            {
                translation.y = -value * (child->computedSize.y - (computedSize.y - scrollBarSection));
            }
            else
            {
                translation.y = -value * (child->computedSize.y - computedSize.y);
            }
            child->Translation(translation);

            m_OnScrollValueChanged(this);
        }
        return *this;
    }


    f32 FScrollBox::NormalizedScrollY()
    {
        FWidget* child = GetChild().Get();
        f32 scrollBarSection = m_ScrollBarMargin * 2 + m_ScrollBarWidth;

        if (child)
        {
            Vec2 translation = child->Translation();
            if (isHorizontalScrollVisible)
            {
                return Math::Clamp01(-translation.y / (child->computedSize.y - (computedSize.y - scrollBarSection)));
            }
            else
            {
                return Math::Clamp01(-translation.y / (child->computedSize.y - computedSize.y));
            }
        }
        return 0;
    }

    void FScrollBox::ClampTranslation()
    {
        FWidget* child = GetChild().Get();
        if (!child)
            return;

        f32 scrollBarSection = m_ScrollBarMargin * 2 + m_ScrollBarWidth;

        Vec2 translation = child->Translation();

        if (isVerticalScrollVisible)
        {
            if (-translation.y < 0)
            {
                translation.y = 0;
            }
            else if (-translation.y > (child->computedSize.y - computedSize.y))
            {
                translation.y = -(child->computedSize.y - computedSize.y);
            }
        }

        if (isHorizontalScrollVisible)
        {
	        if (-translation.x < 0)
	        {
                translation.x = 0;
	        }
            else if (-translation.x > (child->computedSize.x - computedSize.x))
            {
                translation.x = -(child->computedSize.x - computedSize.x);
            }
        }

        if (isVerticalScrollVisible || isHorizontalScrollVisible)
        {
            child->Translation(translation);
        }
    }

    FScrollBox::Self& FScrollBox::NormalizedScrollX(f32 value)
    {
        value = Math::Clamp01(value);
        
        FWidget* child = GetChild().Get();
        f32 scrollBarSection = m_ScrollBarMargin * 2 + m_ScrollBarWidth;

        if (child && isHorizontalScrollVisible)
        {
            Vec2 translation = child->Translation();
            if (isVerticalScrollVisible)
            {
                translation.x = -value * (child->computedSize.x - (computedSize.x - scrollBarSection));
            }
            else
            {
                translation.x = -value * (child->computedSize.x - computedSize.x);
            }
            child->Translation(translation);

            m_OnScrollValueChanged(this);
        }
        return *this;
    }

    f32 FScrollBox::NormalizedScrollX()
    {
        FWidget* child = GetChild().Get();
        f32 scrollBarSection = m_ScrollBarMargin * 2 + m_ScrollBarWidth;

        if (child && isHorizontalScrollVisible)
        {
            Vec2 translation = child->Translation();
            if (isVerticalScrollVisible)
            {
                return Math::Clamp01(-translation.x / (child->computedSize.x - (computedSize.x - scrollBarSection)));
            }
            else
            {
                return Math::Clamp01(-translation.x / (child->computedSize.x - computedSize.x));
            }
        }
        return 0;
    }

}

