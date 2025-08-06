#include "CrystalWidgets.h"

#define TRACY_ZONE\
	ZoneScoped;\
	ZoneTextF(GetName().GetCString());

namespace CE::Widgets
{

    CWindow::CWindow()
    {
        receiveMouseEvents = false;
        clipChildren = true;

        title = GetName().GetString();

        AddDefaultBehavior<CScrollBehavior>();
    }

    CWindow::~CWindow()
    {
        if (nativeWindow)
        {
            delete nativeWindow; nativeWindow = nullptr;
        }
    }

    void CWindow::Show()
    {
	    if (nativeWindow)
	    {
            nativeWindow->Show();
	    }
    }

    void CWindow::Hide()
    {
	    if (nativeWindow)
	    {
            nativeWindow->Hide();
	    }
    }

    void CWindow::SetPlatformWindow(PlatformWindow* window)
    {
        this->nativeWindow = new CPlatformWindow(this, window);

        receiveMouseEvents = true;

        OnPlatformWindowSet();
    }

    void CWindow::UpdateLayoutIfNeeded()
    {
        Super::UpdateLayoutIfNeeded();

    }

    void CWindow::OnAfterUpdateLayout()
    {
        TRACY_ZONE;

	    Super::OnAfterUpdateLayout();

        Vec2 originalSize = GetComputedLayoutSize();
        f32 contentMaxY = originalSize.height;
        f32 contentMaxX = originalSize.width;

        for (CWidget* widget : attachedWidgets)
        {
            if (!widget->IsEnabled())
                continue;

            Vec2 pos = widget->GetComputedLayoutTopLeft();
            Vec2 size = widget->GetComputedLayoutSize();
            f32 maxY = pos.y + size.height;
            f32 maxX = pos.x + size.width;
            if (isnan(maxX))
                maxX = 0;
            if (isnan(maxY))
                maxY = 0;

            contentMaxY = Math::Max(contentMaxY, maxY);
            contentMaxX = Math::Max(contentMaxX, maxX);
        }

        contentSize = Vec2(contentMaxX, contentMaxY);
    }

    void CWindow::OnBeforeDestroy()
    {
        Super::OnBeforeDestroy();

        delete nativeWindow; nativeWindow = nullptr;

        if (!IsDefaultInstance())
        {
            CApplication::Get()->GetFrameScheduler()->WaitUntilIdle();
        }
    }

    void CWindow::Construct()
    {
        Super::Construct();

    }

    Vec2 CWindow::CalculateIntrinsicSize(f32 width, f32 height)
    {
        return Vec2(windowSize.x <= 0 ? YGUndefined : windowSize.x, (allowVerticalScroll || windowSize.y <= 0) ? YGUndefined : windowSize.y);
    }

    bool CWindow::IsDockSpace()
    {
        return IsOfType<CDockSpace>();
    }

    CPlatformWindow* CWindow::GetRootNativeWindow()
    {
        if (nativeWindow)
            return nativeWindow;

        for (CWindow* owner = ownerWindow; owner != nullptr; owner = owner->ownerWindow)
        {
            if (owner->nativeWindow != nullptr)
                return owner->nativeWindow;
        }

        return nullptr;
    }

    static Rect ScaleRect(const Rect& rect, Vec2 scale = Vec2(1, 1))
    {
        Vec2 size = rect.GetSize();
        Vec2 center = (rect.min + rect.max) * 0.5f;
        return Rect(center - size / 2.0f * scale, center + size / 2.0f * scale);
    }

    static Rect ScaleRect(const Rect& rect, f32 scale)
    {
        return ScaleRect(rect, Vec2(scale, scale));
    }

    static Rect TranslateRect(const Rect& rect, Vec2 translation = Vec2(0, 0))
    {
        Vec2 size = rect.GetSize();
        return Rect::FromSize(rect.min + translation, size);
    }

    void CWindow::OnPaint(CPaintEvent* paintEvent)
    {
        Super::OnPaint(paintEvent);

    }

    void CWindow::OnPaintOverlay(CPaintEvent* paintEvent)
    {
        TRACY_ZONE;

        Super::OnPaintOverlay(paintEvent);

        CPainter* painter = paintEvent->painter;

        Color bgColor = computedStyle.properties[CStylePropertyType::Background].color;

        auto style = styleSheet->SelectStyle(this, CStateFlag::Default, CSubControl::TitleBar);

        Color titleBarBgColor = style.GetBackgroundColor();
        if (titleBarBgColor.a > 0)
        {
            bgColor = titleBarBgColor;
        }

        CPen pen = CPen(); pen.SetColor(Color(1, 1, 1, 1)); pen.SetWidth(2.0f);
        CBrush brush = CBrush();

        if (nativeWindow != nullptr && PlatformMisc::GetCurrentPlatform() != PlatformName::Mac &&
            nativeWindow->GetPlatformWindow()->IsBorderless()) // The CWindow is a native window
        {
            painter->SetPen(pen);
            painter->SetBrush(brush);

            auto rootPadding = GetFinalRootPadding();

            Vec2 size = GetComputedLayoutSize() - Vec2(rootPadding.x + rootPadding.z, rootPadding.y + rootPadding.w);
            Vec2 pos = GetComputedLayoutTopLeft() + rootPadding.min;
            constexpr f32 controlWidth = 40;

            controlRects.Resize(3);

            for (int i = 0; i < 3; i++)
            {
                controlRects[i] = Rect::FromSize(pos - rootPadding.min + Vec2(size.width - controlWidth * (3 - i), 0),
                    Vec2(controlWidth, controlWidth));
                // Shrink the rect perpendicular to it's edges
                controlRects[i] = Rect::FromSize(controlRects[i].min + Vec2(controlWidth, controlWidth) * 0.35f,
                    Vec2(controlWidth, controlWidth) * 0.3f);
            }

            if (canBeClosed)
            {
                if (!hoveredControls[2] || clickedControlIdx == 2)
                {
                    pen.SetColor(Color::RGBA8(180, 180, 180));
                    painter->SetPen(pen);
                }
                else
                {
                    pen.SetColor(Color::RGBA8(255, 255, 255));
                    painter->SetPen(pen);
                }

                painter->DrawRoundedX(controlRects[2]);
            }

            pen.SetWidth(1.5f);

            if (canBeMaximized)
            {
                if (!hoveredControls[1] || clickedControlIdx == 1)
                {
                    pen.SetColor(Color::RGBA8(200, 200, 200));
                    painter->SetPen(pen);
                }
                else
                {
                    pen.SetColor(Colors::White);
                    painter->SetPen(pen);
                }

                if (!nativeWindow->GetPlatformWindow()->IsMaximized())
                {
                    painter->DrawRect(ScaleRect(controlRects[1], 0.98f));
                }
                else
                {
                    brush.SetColor(bgColor);
                    painter->SetBrush(brush);

                    auto rectSize = controlRects[1].GetSize();
                    painter->DrawRect(TranslateRect(ScaleRect(controlRects[1], 0.8f), Vec2(1, -1) * 2.0f));
                    painter->DrawRect(ScaleRect(controlRects[1], 0.8f));
                }
            }

            brush.SetColor(Colors::Clear);
            pen.SetColor(Colors::Clear);
            pen.SetWidth(0.0f);

            if (!hoveredControls[0] || clickedControlIdx == 0)
            {
                brush.SetColor(Color::RGBA8(180, 180, 180));
                painter->SetPen(pen);
                painter->SetBrush(brush);
            }
            else
            {
                brush.SetColor(Colors::White);
                painter->SetPen(pen);
                painter->SetBrush(brush);
            }

            if (canBeMinimized)
            {
                Vec2 rectSize = controlRects[0].GetSize();
                painter->DrawRect(Rect::FromSize(Vec2(controlRects[0].min.x, controlRects[0].min.y + rectSize.y / 2 - 2.0f / 2.0f),
                    Vec2(rectSize.width, 2.0f)));
            }
        }
    }

    Rect CWindow::GetVerticalScrollBarRect()
    {
        if (allowVerticalScroll)
        {
            auto app = CApplication::Get();

            Vec2 originalSize = GetComputedLayoutSize();
            f32 originalHeight = originalSize.height;
            f32 contentMaxY = contentSize.height;

            if (contentMaxY > originalHeight + app->styleConstants.scrollSizeBuffer)
            {
                Rect scrollRegion = Rect::FromSize(Vec2(originalSize.width - app->styleConstants.scrollRectWidth, 0), 
                    Vec2(app->styleConstants.scrollRectWidth, originalHeight));
                f32 scrollRectHeightRatio = originalHeight / contentMaxY;

                Rect scrollRect = Rect::FromSize(scrollRegion.min,
                    Vec2(scrollRegion.GetSize().width, Math::Max(scrollRegion.GetSize().height * scrollRectHeightRatio, 
                        app->styleConstants.minScrollRectSize)));
                scrollRect = scrollRect.Translate(Vec2(0, (originalHeight - scrollRect.GetSize().height) * normalizedScroll.y));

                return scrollRect;
            }
        }

        return Rect();
    }

    bool CWindow::WindowHitTest(PlatformWindow* window, Vec2 position)
    {
        if (controlRects.NonEmpty() && nativeWindow != nullptr)
        {
            f32 minX = UINT_MAX;

            for (int i = 0; i < controlRects.GetSize(); ++i)
            {
                if (i == 0 && !canBeMinimized)
                    continue;
                if (i == 1 && !canBeMaximized)
                    continue;
                if (i == 2 && !canBeClosed)
                    continue;

                minX = Math::Min(minX, controlRects[i].min.x);
            }

            if (position.y < rootPadding.y && position.x < minX)
            {
	            return true;
            }
        }

        return false;
    }

    void CWindow::HandleEvent(CEvent* event)
    {
        TRACY_ZONE;

        if (allowVerticalScroll || allowHorizontalScroll)
            receiveDragEvents = true;

        auto app = CApplication::Get();

        if (event->IsMouseEvent())
        {
            CMouseEvent* mouseEvent = static_cast<CMouseEvent*>(event);
            Vec2 globalMousePos = mouseEvent->mousePos;
            Rect screenSpaceWindowRect = GetScreenSpaceRect();
            Vec2 windowSpaceMousePos = globalMousePos - screenSpaceWindowRect.min;

            // Window controls on top-right corner

            if (controlRects.NonEmpty() && nativeWindow != nullptr)
            {
                if (mouseEvent->type == CEventType::MousePress && mouseEvent->button == MouseButton::Left)
                {
                    for (int i = 0; i < controlRects.GetSize(); ++i)
                    {
                        if (controlRects[i].Contains(windowSpaceMousePos))
                        {
                            clickedControlIdx = i;
                        }
                    }
                }
                else if (mouseEvent->type == CEventType::MouseRelease && mouseEvent->button == MouseButton::Left)
                {
	                for (int i = 0; i < controlRects.GetSize(); ++i)
	                {
		                if (controlRects[i].Contains(windowSpaceMousePos))
		                {
                            if (i == clickedControlIdx)
                            {
                                if (clickedControlIdx == 0)
                                    nativeWindow->GetPlatformWindow()->Minimize();
                                else if (clickedControlIdx == 1 && nativeWindow->GetPlatformWindow()->IsMaximized())
                                    nativeWindow->GetPlatformWindow()->Restore();
                                else if (clickedControlIdx == 1)
                                    nativeWindow->GetPlatformWindow()->Maximize();
                                else if (clickedControlIdx == 2) // Close button clicked
                                {
                                    OnClickClose();
	                                QueueDestroy();
                                }
                            }
		                }
	                }

                    clickedControlIdx = -1;
                }
                else if (mouseEvent->type == CEventType::MouseMove)
                {
                    for (int i = 0; i < controlRects.GetSize(); ++i)
                    {
                        if (controlRects[i].Contains(windowSpaceMousePos))
                        {
                            hoveredControls[i] = true;
                            SetNeedsPaint();
                        }
                        else
                        {
                            if (hoveredControls[i])
                                SetNeedsPaint();
                            hoveredControls[i] = false;
                        }
                    }
                }
                else if (mouseEvent->type == CEventType::MouseLeave)
                {
                    for (int i = 0; i < controlRects.GetSize(); ++i)
                    {
                        if (hoveredControls[i])
                            SetNeedsPaint();
                        hoveredControls[i] = false;
                    }
                }
            }
        }

        Super::HandleEvent(event);
    }

    bool CWindow::IsSubWidgetAllowed(Class* subWidgetClass)
    {
        return subWidgetClass->IsSubclassOf<CWidget>() && !subWidgetClass->IsSubclassOf<CMenuItem>();
    }

    Vec2 CWindow::GetWindowSize() const
    {
        if (nativeWindow)
        {
            return nativeWindow->GetPlatformWindow()->GetWindowSize().ToVec2();
        }
        return windowSize;
    }

    bool CWindow::IsViewportWindow() const
    {
        return IsOfType<CViewport>();
    }

    bool CWindow::IsAlwaysOnTop() const
    {
        if (nativeWindow)
        {
            return nativeWindow->IsAlwaysOnTop();
        }
        return false;
    }

    void CWindow::SetAlwaysOnTop(bool set)
    {
        if (nativeWindow)
        {
            nativeWindow->SetAlwaysOnTop(set);
        }
    }

    CWidget* CWindow::HitTest(Vec2 windowSpaceMousePos)
    {
        bool shouldBreak = false;

        for (int i = attachedWindows.GetSize() - 1; !shouldBreak && i >= 0; i--)
        {
            CWindow* attachedWindow = attachedWindows[i];
            if (!attachedWindow->IsEnabled() || !attachedWindow->IsVisible())
            {
	            continue;
            }

            if (attachedWindow->IsOfType<CPopup>())
            {
                CPopup* popup = static_cast<CPopup*>(attachedWindow);
                if (popup->blockHitTest)
                {
                    shouldBreak = true;
                }
            }

	        CWidget* widget = attachedWindow->HitTest(windowSpaceMousePos);

            if (widget)
            {
                return widget;
            }
        }

	    return Super::HitTest(windowSpaceMousePos);
    }

    void CWindow::AttachSubWindow(CWindow* subWindow)
    {
        if (attachedWindows.Exists(subWindow))
            return;

        attachedWindows.Add(subWindow);

        subWindow->SetNeedsPaint();
        SetNeedsPaint();
    }

    void CWindow::DetachSubWindow(CWindow* subWindow)
    {
        attachedWindows.Remove(subWindow);

        SetNeedsPaint();
    }

    void CWindow::OnClickClose()
    {

    }

    void CWindow::OnPlatformWindowSet()
    {

    }

    void CWindow::OnSubobjectAttached(Object* object)
    {
        Super::OnSubobjectAttached(object);

        if (!object)
            return;

        if (object->IsOfType<CWidget>())
        {
            ((CWidget*)object)->ownerWindow = this;
        }

        if (object->IsOfType<CMenuItem>())
        {
            menuItems.Add(static_cast<CMenuItem*>(object));
        }
    }

    void CWindow::OnSubobjectDetached(Object* object)
    {
        Super::OnSubobjectDetached(object);
        
        if (!object)
            return;

        if (object->IsOfType<CWidget>())
        {
            ((CWidget*)object)->ownerWindow = nullptr;
        }

    	if (object->IsOfType<CMenuItem>())
        {
            menuItems.Remove(static_cast<CMenuItem*>(object));
        }
    }

} // namespace CE::Widgets
