#include "CrystalWidgets.h"

namespace CE::Widgets
{
    constexpr f32 majorTabHeight = 40.0f;
    constexpr f32 majorTabOffset = 25.0f;
    constexpr f32 majorTabWidth = 270.0f;
    constexpr f32 minorTabHeight = 27.0f;
    constexpr f32 minorTabOffset = 2.0f;
    constexpr f32 minorTabWidth = 200.0f;
    constexpr f32 splitterWidth = 2.0f;

    CDockSplitView::CDockSplitView()
    {
        receiveDragEvents = true;
    }

    CDockSplitView::~CDockSplitView()
    {

    }

    CDockSplitView* CDockSplitView::FindSplit(Name splitName)
    {
        if (GetName() == splitName)
            return this;

        for (auto child : children)
        {
            CDockSplitView* result = child->FindSplit(splitName);
            if (result)
            {
                return result;
            }
        }

        return nullptr;
    }

    CDockWindow* CDockSplitView::GetActiveWindow()
    {
        if (selectedTab < 0 || selectedTab >= dockedWindows.GetSize())
            return nullptr;

        return dockedWindows[selectedTab];
    }

    bool CDockSplitView::IsSubWidgetAllowed(Class* subWidgetClass)
    {
        if (!subWidgetClass)
            return false;
        if (subWidgetClass->IsSubclassOf<CDockWindow>())
            return true;
        if (dockSpace->GetDockType() == CDockType::Minor && subWidgetClass->IsSubclassOf<CDockSplitView>())
            return true;
        return false;
    }

    Vec2 CDockSplitView::GetComputedLayoutTopLeft()
    {
        if (parentSplitView == nullptr || dockSpace->GetDockType() != CDockType::Minor)
            return Super::GetComputedLayoutTopLeft();

        Vec2 pos = Vec2();
        Vec2 parentSize = parentSplitView->GetComputedLayoutSize();
        int index = parentSplitView->children.IndexOf(this);
        int numChildren = parentSplitView->children.GetSize();
        if (index < 0)
            return Super::GetComputedLayoutTopLeft();
        CDockSplitView* prevSplitView = nullptr;
    
        for (int i = 0; i < index; i++)
        {
            if (parentSplitView->GetSplitDirection() == CDockSplitDirection::Horizontal)
                pos.x += parentSplitView->children[i]->splitRatio * parentSize.width + splitterWidth;
            else
                pos.y += parentSplitView->children[i]->splitRatio * parentSize.height + splitterWidth;
        }

        return pos;
    }

    Vec2 CDockSplitView::GetComputedLayoutSize()
    {
        if (parentSplitView == nullptr || dockSpace->GetDockType() != CDockType::Minor)
            return Super::GetComputedLayoutSize();

        Vec2 parentSize = parentSplitView->GetComputedLayoutSize();
        int index = parentSplitView->children.IndexOf(this);
        int numChildren = parentSplitView->children.GetSize();
        if (index < 0)
            return Super::GetComputedLayoutSize();

        if (parentSplitView->GetSplitDirection() == CDockSplitDirection::Horizontal)
            return Vec2(parentSize.x * splitRatio - (numChildren - 1) * splitterWidth, parentSize.y);

        return Vec2(parentSize.x, parentSize.y * splitRatio - (numChildren - 1) * splitterWidth);
    }

    void CDockSplitView::SetAutoHideTabs(bool set)
    {
        autoHideTabs = set;
        if (!autoHideTabs || dockedWindows.GetSize() > 1)
        {
            if (dockSpace->GetDockType() == CDockType::Major)
                rootPadding = Vec4(0, 60, 0, 0);
            else
                rootPadding = Vec4(0, 27, 0, 0);
        }
        else
        {
            rootPadding = Vec4();
        }

        SetNeedsStyle();
        SetNeedsLayout();
        SetNeedsPaint();
    }

    void CDockSplitView::OnSubobjectAttached(Object* subobject)
    {
        Super::OnSubobjectAttached(subobject);

        if (subobject->IsOfType<CDockSplitView>())
        {
            CDockSplitView* splitView = (CDockSplitView*)subobject;
            splitView->dockSpace = dockSpace;
            children.Add(splitView);
            splitView->parentSplitView = this;
            rootPadding = Vec4(0, 0, 0, 0);

            allowVerticalScroll = false;
            allowHorizontalScroll = false;
        }
        else if (subobject->IsOfType<CDockWindow>())
        {
            CDockWindow* dockWindow = (CDockWindow*)subobject;
            dockWindow->dockSpace = dockSpace;
            dockWindow->SetEnabled(selectedTab == dockedWindows.GetSize());
            if (dockSpace->GetDockType() == CDockType::Major)
                dockWindow->allowVerticalScroll = dockWindow->allowHorizontalScroll = false;
            dockedWindows.Add(dockWindow);

            selectedTab = 0;
            if (dockSpace->GetDockType() == CDockType::Major)
                rootPadding = Vec4(0, 60, 0, 0);
            else
                rootPadding = Vec4(0, 27, 0, 0);

            allowVerticalScroll = false;
            allowHorizontalScroll = false;
        }
    }

    void CDockSplitView::OnSubobjectDetached(Object* subobject)
    {
        Super::OnSubobjectDetached(subobject);

        if (subobject->IsOfType<CDockSplitView>())
        {
            CDockSplitView* splitView = (CDockSplitView*)subobject;
            splitView->dockSpace = nullptr;
            children.Remove(splitView);
            splitView->parentSplitView = nullptr;
        }
        else if (subobject->IsOfType<CDockWindow>())
        {
            CDockWindow* dockWindow = (CDockWindow*)subobject;
            dockWindow->dockSpace = nullptr;
            dockedWindows.Remove(dockWindow);
        }
    }

    void CDockSplitView::HandleEvent(CEvent* event)
    {
        bool areTabsVisible = true;
        if (autoHideTabs && dockedWindows.GetSize() <= 1)
            areTabsVisible = false;

        if (event->IsMouseEvent() || event->IsDragEvent())
        {
            CMouseEvent* mouseEvent = (CMouseEvent*)event;

            if (event->type == CEventType::MousePress && children.GetSize() > 1 && mouseEvent->button == MouseButton::Left)
            {
                int splitIdx = -1;

                for (int i = 0; i < children.GetSize() - 1; i++)
                {
                    Rect childRect = children[i]->GetScreenSpaceRect();

                    if (splitDirection == CDockSplitDirection::Horizontal)
                    {
                        Rect splitterRect = Rect::FromSize(childRect.min + Vec2(childRect.GetSize().width, 0),
                            Vec2(splitterWidth * 2, childRect.GetSize().height));

                        if (splitterRect.Contains(mouseEvent->mousePos))
                        {
                            splitIdx = i;
                            break;
                        }
                    }
                    else if (splitDirection == CDockSplitDirection::Vertical)
                    {
                        Rect splitterRect = Rect::FromSize(childRect.min + Vec2(0, childRect.GetSize().height),
                            Vec2(childRect.GetSize().width, splitterWidth * 2));

                        if (splitterRect.Contains(mouseEvent->mousePos))
                        {
                            splitIdx = i;
                            break;
                        }
                    }
                }

                if (splitIdx >= 0)
                {
                    draggedSplitIdx = splitIdx;
                }
            }
            else if (event->type == CEventType::MouseMove && children.GetSize() > 1)
            {
                if (draggedSplitIdx >= 0 && mouseEvent->button == MouseButton::Left) // User is dragging the splitter
                {
                    Vec2 mouseDelta = mouseEvent->mousePos - mouseEvent->prevMousePos;
                    if (splitDirection == CDockSplitDirection::Horizontal)
                    {
                        mouseDelta.x /= GetComputedLayoutSize().x;
                        for (int i = 0; i < children.GetSize(); i++)
                        {
	                        if (i == draggedSplitIdx && i < children.GetSize() - 1)
	                        {
                                children[i]->splitRatio += mouseDelta.x;
                                children[i]->splitRatio = Math::Clamp(children[i]->splitRatio, 0.1f, 0.9f);
                                children[i + 1]->splitRatio -= mouseDelta.x;
                                children[i + 1]->splitRatio = Math::Clamp(children[i + 1]->splitRatio, 0.1f, 0.9f);
                                SetNeedsStyle();
	                        	SetNeedsLayout();
                                SetNeedsPaint();
		                        break;
	                        }
                        }
                    }
                    else
                    {
                        mouseDelta.y /= GetComputedLayoutSize().y;
                        for (int i = 0; i < children.GetSize(); i++)
                        {
                            if (i == draggedSplitIdx && i < children.GetSize() - 1)
                            {
                                children[i]->splitRatio += mouseDelta.y;
                                children[i]->splitRatio = Math::Clamp(children[i]->splitRatio, 0.1f, 0.9f);
                                children[i + 1]->splitRatio -= mouseDelta.y;
                                children[i + 1]->splitRatio = Math::Clamp(children[i + 1]->splitRatio, 0.1f, 0.9f);
                                SetNeedsStyle();
                                SetNeedsLayout();
                                SetNeedsPaint();
                                break;
                            }
                        }
                    }
                }
                else if (mouseEvent->button == MouseButton::None) // User is just hovering over the splitter without left mouse button held
                {
                    hoveredSplitIdx = -1;

                    for (int i = 0; i < children.GetSize() - 1; i++)
                    {
                        Rect childRect = children[i]->GetScreenSpaceRect();

                        if (splitDirection == CDockSplitDirection::Horizontal)
                        {
                            Rect splitterRect = Rect::FromSize(childRect.min + Vec2(childRect.GetSize().width, 0),
                                Vec2(splitterWidth * 2, childRect.GetSize().height));

                            if (splitterRect.Contains(mouseEvent->mousePos))
                            {
                                hoveredSplitIdx = i;
                                break;
                            }
                        }
                        else if (splitDirection == CDockSplitDirection::Vertical)
                        {
                            Rect splitterRect = Rect::FromSize(childRect.min + Vec2(0, childRect.GetSize().height),
                                Vec2(childRect.GetSize().width, splitterWidth * 2));

                            if (splitterRect.Contains(mouseEvent->mousePos))
                            {
                                hoveredSplitIdx = i;
                                break;
                            }
                        }
                    }

                    if (hoveredSplitIdx >= 0)
                    {
                        if (hasPushedResizeCursor)
                        {
                            if ((splitDirection == CDockSplitDirection::Horizontal && CApplication::Get()->GetTopCursor() != CCursor::SizeH) ||
                                (splitDirection == CDockSplitDirection::Vertical && CApplication::Get()->GetTopCursor() != CCursor::SizeV))
                            {
                                CApplication::Get()->PopCursor();
                                hasPushedResizeCursor = false;
                            }
                        }
                        if (!hasPushedResizeCursor)
                        {
                            CApplication::Get()->PushCursor(splitDirection == CDockSplitDirection::Horizontal ? CCursor::SizeH : CCursor::SizeV);
                            hasPushedResizeCursor = true;
                        }
                    }
                    else
                    {
                        if (hasPushedResizeCursor)
                        {
                            CApplication::Get()->PopCursor();
                            hasPushedResizeCursor = false;
                        }
                    }
                }
            }
            else if (event->type == CEventType::MouseLeave && children.GetSize() > 1)
            {
                if (hasPushedResizeCursor && draggedSplitIdx < 0)
                {
                    CApplication::Get()->PopCursor();
                    hasPushedResizeCursor = false;
                }
            }
            else if (event->type == CEventType::MouseRelease)
            {
                if (hasPushedResizeCursor)
                {
                    CApplication::Get()->PopCursor();
                    hasPushedResizeCursor = false;
                }
                draggedSplitIdx = -1;
            }
            
            if (!event->isConsumed && areTabsVisible)
            {
                PlatformWindow* platformWindow = dockSpace->GetRootNativeWindow()->GetPlatformWindow();
                Vec2 windowPos = platformWindow->GetWindowPosition().ToVec2();
                Vec2i windowSize = platformWindow->GetWindowSize();
                platformWindow->GetWindowSize();
                Vec2 pos = mouseEvent->mousePos - windowPos;
                Vec2 prevPos = mouseEvent->prevMousePos - windowPos;
                Vec2 delta = pos - prevPos;
                Rect screenSpaceRect = GetScreenSpaceRect();

                if (event->type == CEventType::DragBegin && event->sender == this)
                {
                    totalDeltaX = 0;

                    for (int i = 0; i < tabs.GetSize(); ++i)
                    {
                        Rect tabRect = tabs[i].rect;
                        auto tabSize = tabRect.GetSize();
                        tabRect.min += rootOrigin;
                        tabRect.max = tabRect.min + tabSize;
                        if (tabRect.Contains(pos))
                        {
                            if (selectedTab != i)
                            {
                                SetNeedsPaint();
                            }
                            selectedTab = i;
                            event->Consume(this); // The user clicked inside a tab
                            break;
                        }
                    }

                    for (int i = 0; i < tabs.GetSize(); ++i)
                    {
                        GetSubWidget(i)->SetEnabled(selectedTab == i);
                    }
                }
                else if (event->type == CEventType::DragMove)
                {
                    CDragEvent* dragEvent = (CDragEvent*)event;

                    if (dockedWindows.GetSize() > 0)
                    {
                        dragEvent->Consume(this);

                        for (int i = 0; i < tabs.GetSize(); ++i)
                        {
                            if (i == selectedTab)
                            {
                                Vec2 size = tabs[i].rect.GetSize();
                                tabs[i].rect.min.x += delta.x;
                                totalDeltaX += delta.x;
                                tabs[i].rect.max = tabs[i].rect.min + size;
                                f32 nextMidPoint = (tabs[i].rect.min.x + size.width * 0.5f);
                                f32 prevMidPoint = (tabs[i].rect.min.x + size.width * 0.5f);
                                //if (i < tabs.GetSize() - 1)
                                if (false) // TODO: Tab reordering logic
                                {
                                    f32 nextStartPoint = tabs[i + 1].rect.min.x;
                                    if (nextMidPoint > nextStartPoint)
                                    {
                                        std::swap(tabs[selectedTab], tabs[selectedTab + 1]);
                                        std::swap(attachedWidgets[selectedTab], attachedWidgets[selectedTab + 1]);
                                        std::swap(dockedWindows[selectedTab], dockedWindows[selectedTab + 1]);
                                        Vec2 prevSize = tabs[selectedTab].rect.GetSize();
                                        tabs[selectedTab].rect.min.x = tabs[selectedTab + 1].rect.min.x - totalDeltaX;
                                        tabs[selectedTab].rect.max.x = tabs[selectedTab].rect.min.x + prevSize.x;
                                        selectedTab++;
                                        totalDeltaX = 0;
                                    }
                                }

                                SetNeedsPaint();
                                break;
                            }
                        }
                    }
                }
                else if (event->type == CEventType::DragEnd)
                {
                    event->Consume(this);
                    totalDeltaX = 0;

                    tabs.Clear();
                    SetNeedsPaint();
                    CDragEvent* dragEvent = (CDragEvent*)event;
                }
            }
        }

	    Super::HandleEvent(event);
    }

    void CDockSplitView::OnPaint(CPaintEvent* paintEvent)
    {
        Super::OnPaint(paintEvent);

        CPainter* painter = paintEvent->painter;

        Color bgColor = computedStyle.properties[CStylePropertyType::Background].color;

        CPen pen = CPen(Color::RGBA8(48, 48, 48));
        CBrush brush = CBrush(bgColor);//CBrush(Color::FromRGBA32(21, 21, 21));
        CFont font = CFont("Roboto", 15, false);

        bool areTabsVisible = true;
        if (autoHideTabs && dockedWindows.GetSize() <= 1)
            areTabsVisible = false;

        // - Draw Tabs -
        tabs.Clear();

        if (children.IsEmpty())
        {
            Vec2 baseOffset = {};
            if (parentSplitView != nullptr && dockSpace->GetDockType() == CDockType::Minor)
            {
                int index = parentSplitView->children.IndexOf(this);
                Vec2 parentSize = parentSplitView->GetComputedLayoutSize();
                baseOffset.x -= 20.5f;

                for (int i = 0; i < index; ++i)
                {
	                if (parentSplitView->GetSplitDirection() == CDockSplitDirection::Horizontal)
		                baseOffset.x += parentSplitView->children[i]->splitRatio * parentSize.width;
	                else
		                baseOffset.y += parentSplitView->children[i]->splitRatio * parentSize.height;
                }
            }

            f32 startXOffset = 20.0f + baseOffset.x;
            const f32 startYOffset = baseOffset.y;

            f32 xOffset = startXOffset;

            int tabsCount = tabs.GetSize();
            if (tabsCount < GetSubWidgetCount())
            {
                tabs.Resize(GetSubWidgetCount());

                for (int i = 0; i < tabs.GetSize(); ++i)
                {
                    GetSubWidget(i)->SetEnabled(selectedTab == i);
                }
            }

            f32 tabOffsetY = majorTabOffset;
            f32 tabHeight = majorTabHeight;
            f32 maxTabWidth = majorTabWidth;
            if (dockSpace->GetDockType() == CDockType::Minor)
            {
                tabOffsetY = minorTabOffset;
                tabHeight = minorTabHeight;
                maxTabWidth = minorTabWidth;
                font.SetSize(14);
            }

            tabOffsetY += startYOffset;

            if (areTabsVisible)
            {
                // Draw non-selected tabs first
                for (int i = 0; i < GetSubWidgetCount(); ++i)
                {
                    CWidget* subWidget = GetSubWidget(i);

                    if (subWidget->IsOfType<CDockWindow>())
                    {
                        CDockWindow* dockWindow = (CDockWindow*)subWidget;

                        Vec2 tabTitleSize = painter->CalculateTextSize(dockWindow->GetTitle());

                        pen.SetWidth(0.0f);
                        pen.SetColor(Colors::Clear);
                        brush.SetColor(Color::RGBA8(36, 36, 36));
                        if (i != selectedTab)
                            brush.SetColor(bgColor);
                        painter->SetPen(pen); painter->SetBrush(brush); painter->SetFont(font);

                        if (i >= tabsCount)
                        {
                            Rect rect = Rect::FromSize(xOffset, tabOffsetY, Math::Min(tabTitleSize.width + 70, maxTabWidth), tabHeight);
                            tabs[i] = { rect };
                        }

                        Rect tabRect = tabs[i].rect;

                        if (selectedTab != i)
                        {
                            painter->DrawRoundedRect(tabRect, Vec4(5, 5, 0, 0));

                            pen.SetColor(Colors::White);
                            painter->SetPen(pen);

                            painter->DrawText(dockWindow->GetTitle(), tabRect + Rect(15, tabRect.GetSize().height / 2 - tabTitleSize.height / 2, 0, 0));
                        }

                        xOffset += tabRect.GetSize().width + 2.5f;
                    }
                }

                // Draw selected tab at last
                xOffset = startXOffset;

                // Draw selected tab
                for (int i = 0; i < GetSubWidgetCount(); ++i)
                {
                    CWidget* subWidget = GetSubWidget(i);

                    if (subWidget->IsOfType<CDockWindow>())
                    {
                        CDockWindow* dockWindow = (CDockWindow*)subWidget;

                        Vec2 tabTitleSize = painter->CalculateTextSize(dockWindow->GetTitle());

                        pen.SetWidth(0.0f);
                        pen.SetColor(Colors::Clear);
                        brush.SetColor(Color::RGBA8(36, 36, 36));
                        if (i != selectedTab)
                            brush.SetColor(bgColor);
                        painter->SetPen(pen); painter->SetBrush(brush); painter->SetFont(font);

                        Rect tabRect = tabs[i].rect;

                        if (selectedTab == i)
                        {
                            painter->DrawRoundedRect(tabRect, Vec4(5, 5, 0, 0));

                            pen.SetColor(Colors::White);
                            painter->SetPen(pen);

                            painter->DrawText(dockWindow->GetTitle(), tabRect + Rect(15, tabRect.GetSize().height / 2 - tabTitleSize.height / 2, 0, 0));
                        }

                        xOffset += tabRect.GetSize().width + 2.5f;
                    }
                }

            }
        }
        else
        {
            // Children dock splits exist

        }
    }

} // namespace CE::Widgets
