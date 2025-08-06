#include "CrystalWidgets.h"

namespace CE::Widgets
{

	CDockSpace::CDockSpace()
	{
        allowVerticalScroll = allowHorizontalScroll = false;
        receiveMouseEvents = true;
        rootPadding = Vec4(1, 1, 1, 1) * 1.5f;

        CDockSplitView* full = CreateDefaultSubobject<CDockSplitView>("DockSplitView");
        full->dockSpace = this;
        full->splitRatio = 1.0f;

        dockSplits.Add(full);
	}

	CDockSpace::~CDockSpace()
	{
        if (nativeWindow)
        {
            nativeWindow->GetPlatformWindow()->SetHitTestDelegate(nullptr);
        }
	}

	CDockSplitView* CDockSpace::GetRootDockSplit() const
	{
        if (dockSplits.IsEmpty())
            return nullptr;
        return dockSplits[0];
	}

    bool CDockSpace::Split(CDockSplitView* originalSplit, f32 splitRatio, CDockSplitDirection splitDirection, Name splitName1, Name splitName2)
    {
        if (dockType == CDockType::Major)
            return false;

        splitRatio = Math::Clamp(splitRatio, 0.05f, 0.95f);

        if (!splitName1.IsValid())
            splitName1 = "DockSplitView";
        if (!splitName2.IsValid())
            splitName2 = "DockSplitView";

        CDockSplitView* first = CreateObject<CDockSplitView>(originalSplit, splitName1.GetString());
        CDockSplitView* second = CreateObject<CDockSplitView>(originalSplit, splitName2.GetString());

        first->splitRatio = 1.0f - splitRatio;
        second->splitRatio = splitRatio;
        first->dockSpace = second->dockSpace = this;

        originalSplit->splitDirection = splitDirection;

        return true;
    }

    bool CDockSpace::Split(f32 splitRatio, CDockSplitDirection splitDirection, Name splitName1, Name splitName2)
    {
        if (dockType == CDockType::Major)
            return false;

        CDockSplitView* originalSplit = dockSplits.Top();
        return Split(originalSplit, splitRatio, splitDirection, splitName1, splitName2);
    }

    void CDockSpace::HandleEvent(CEvent* event)
    {
        ZoneScoped;

        if (dockType == CDockType::Major && dockSplits.NonEmpty())
        {
            if (event->IsMouseEvent())
            {
                CMouseEvent* mouseEvent = static_cast<CMouseEvent*>(event);
                Vec2 mousePos = mouseEvent->mousePos;

                if (event->type == CEventType::MouseEnter || event->type == CEventType::MouseMove)
                {
                    int hoveredItem = -1;

                    for (int i = 0; i < menuItemRects.GetSize(); ++i)
                    {
                        Rect menuItemRect = LocalToScreenSpaceRect(menuItemRects[i]);

                        if (menuItemRect.Contains(mousePos))
                        {
                            hoveredItem = i;
                        }
                    }

                    if (hoveredMenuItem != hoveredItem)
                    {
                        hoveredMenuItem = hoveredItem;
                        if (activeMenuItem >= 0 && hoveredMenuItem >= 0 && activeMenuItem != hoveredMenuItem)
                        {
                            activeMenuItem = hoveredMenuItem;

                            if (curMenuPopup != nullptr && curMenuPopup->IsShown())
                            {
                                CWindow* dockWindow = dockSplits[0]->GetActiveWindow();
                                curMenuPopup->Hide();

                                CMenu* menuPopup = dockWindow->menuItems[activeMenuItem]->GetSubMenu();
                                Vec2 pos = menuItemRects[activeMenuItem].min + Vec2(0, menuItemRects[activeMenuItem].GetSize().height);

                                menuPopup->UpdateStyleIfNeeded();
                                menuPopup->UpdateLayoutIfNeeded();

                                Vec2 menuSize = menuPopup->GetComputedLayoutSize();
                                menuSize = Vec2();
                                Vec2 screenSpacePos = LocalToScreenSpacePos(pos);

                                menuPopup->Show(Vec2i((int)screenSpacePos.x, (int)screenSpacePos.y), menuSize.ToVec2i());

                                curMenuPopup = menuPopup;
                            }
                            else if (curMenuPopup != nullptr)
                            {
                                activeMenuItem = -1;
                                hoveredMenuItem = -1;
                                curMenuPopup = nullptr;
                            }
                        }
                        SetNeedsPaint();
                    }
                }
                else if (event->type == CEventType::MouseLeave)
                {
                    hoveredMenuItem = -1;
                    SetNeedsPaint();
                }
                else if (event->type == CEventType::MousePress)
                {
                    CWindow* dockWindow = dockSplits[0]->GetActiveWindow();

                    if (hoveredMenuItem >= 0 && activeMenuItem != hoveredMenuItem && mouseEvent->button == MouseButton::Left)
                    {
                        activeMenuItem = hoveredMenuItem;
                        CMenu* menuPopup = dockWindow->menuItems[activeMenuItem]->GetSubMenu();
                        Vec2 pos = menuItemRects[activeMenuItem].min + Vec2(0, menuItemRects[activeMenuItem].GetSize().height);

                        menuPopup->UpdateStyleIfNeeded();
                        menuPopup->UpdateLayoutIfNeeded();

                        Vec2 menuSize = menuPopup->GetComputedLayoutSize();
                        menuSize = Vec2();
                        Vec2 screenSpacePos = LocalToScreenSpacePos(pos);

                        menuPopup->Show(Vec2i((int)screenSpacePos.x, (int)screenSpacePos.y), menuSize.ToVec2i());

                        curMenuPopup = menuPopup;
                    }
                    else
                    {
                        if (curMenuPopup)
                        {
                            curMenuPopup->Hide();
                            curMenuPopup = nullptr;
                        }

                        activeMenuItem = -1;
                    }

                    SetNeedsPaint();
                }
                else if (event->type == CEventType::MouseRelease)
                {
                    
                }
            }
        }
        
	    Super::HandleEvent(event);
    }

    void CDockSpace::OnPlatformWindowSet()
    {
        Super::OnPlatformWindowSet();

        if (nativeWindow)
        {
	        nativeWindow->GetPlatformWindow()->SetHitTestDelegate(MemberDelegate(&Self::WindowHitTest, this));
        }
    }

    void CDockSpace::OnSubobjectAttached(Object* object)
	{
		Super::OnSubobjectAttached(object);

        if (!object)
            return;

        if (object->IsOfType<CWindow>())
        {
            CWindow* window = (CWindow*)object;

            dockSplits.Top()->AddSubWidget(window);
        }
        else if (object->IsOfType<CDockSplitView>())
        {
            CDockSplitView* splitView = (CDockSplitView*)object;
            splitView->parent = this;
            attachedWidgets.Add(splitView);
        }
	}

	void CDockSpace::OnSubobjectDetached(Object* object)
	{
        Super::OnSubobjectDetached(object);

        if (!object)
            return;

        if (object->IsOfType<CWindow>())
        {
            CWindow* window = (CWindow*)object;

            for (CDockSplitView* dockSplit : dockSplits)
            {
                dockSplit->RemoveSubWidget(window);
            }
        }
        else if (object->IsOfType<CDockSplitView>())
        {
            CDockSplitView* splitView = (CDockSplitView*)object;
            splitView->parent = nullptr;
            attachedWidgets.Remove(splitView);
        }
	}

	void CDockSpace::OnPaint(CPaintEvent* paintEvent)
	{
        ZoneScoped;

		Super::OnPaint(paintEvent);

        CPainter* painter = paintEvent->painter;

        Color bgColor = computedStyle.properties[CStylePropertyType::Background].color;

        CPen pen = CPen(Color::RGBA8(60, 60, 60));
        CBrush brush = CBrush(Colors::Clear);
        CFont font = CFont("Roboto", 15, false);

        if (GetDockType() == CDockType::Major) // Major dock space
        {
        	PlatformWindow* nativeWindow = GetRootNativeWindow()->GetPlatformWindow();

            u32 w = 0, h = 0;
            nativeWindow->GetDrawableWindowSize(&w, &h);

            // - Draw Background -

            painter->SetBrush(brush);

            painter->DrawRect(Rect::FromSize(0, 0, w, h));

            if (nativeWindow != nullptr && nativeWindow->IsBorderless() && !nativeWindow->IsMaximized() && !nativeWindow->IsFullscreen())
            {
                pen = CPen(Color::RGBA(15, 15, 15));
                pen.SetWidth(2.0f);

                painter->SetBrush(CBrush());
                painter->SetPen(pen);

                nativeWindow->GetWindowSize(&w, &h);

                painter->DrawRect(Rect::FromSize(Vec2(), Vec2(w, h)));

                pen.SetColor(Color::RGBA(50, 50, 50));
                pen.SetWidth(1.0f);
                painter->SetPen(pen);

                painter->DrawRect(Rect::FromSize(Vec2(2, 2), Vec2(w - 4, h - 4)));
            }

            // Draw menu items

            if (curMenuPopup != nullptr && !curMenuPopup->IsShown())
            {
                activeMenuItem = -1;
                curMenuPopup = nullptr;
            }

            if (dockSplits.NonEmpty() && dockSplits[0]->GetSubWidgetCount() > 0 && dockSplits[0]->GetSubWidget(0)->IsOfType<CWindow>())
            {
                CWindow* dockWindow = dockSplits[0]->GetActiveWindow();

                f32 posX = 20.0f;

                menuItemRects.Clear();
                menuItemRects.Reserve(dockWindow->menuItems.GetSize());

                for (int i = 0; i < dockWindow->menuItems.GetSize(); ++i)
                {
                    CMenuItem* menuItem = dockWindow->menuItems[i];

                    menuItem->UpdateStyleIfNeeded();

                    font = CFont(menuItem->computedStyle.GetFontName(), menuItem->computedStyle.GetFontSize());
                    painter->SetFont(font);

                    Vec2 textSize = painter->CalculateTextSize(menuItem->GetText());
                    const Vec2 padding = Vec2(10, 4);

                    Rect rect = Rect::FromSize(posX, 2.0f, padding.x * 2 + textSize.width, padding.y * 2 + textSize.height);
                    menuItemRects.Add(rect);

                    Color textColor = Colors::White;
                    pen = CPen(textColor);
                    painter->SetPen(pen);

                    if (i == hoveredMenuItem || i == activeMenuItem)
                    {
                        // TODO: Use style sheet for fetching these colors
                        Color menuItemColor = i == activeMenuItem ? Color::RGBA(0, 112, 224) : Color::RGBA(87, 87, 87);

                        painter->SetBrush(CBrush(menuItemColor));

                        painter->DrawRect(rect);
                    }

                    painter->DrawText(menuItem->GetText(), padding + Vec2(posX, 2.0f));

                    posX += rect.GetSize().width;
                }
            }
        }
        else if (GetDockType() == CDockType::Minor)
        {
            Rect rect = Rect::FromSize(GetComputedLayoutTopLeft(), GetComputedLayoutSize());

            painter->SetBrush(brush);

            painter->DrawRect(rect);
        }
	}

    bool CDockSpace::WindowHitTest(PlatformWindow* window, Vec2 position)
    {
        if (dockSplits.IsEmpty()) // Should never happen
        {
            if (position.y < 60)
                return true;
            return false;
        }

        for (const auto& tab : dockSplits[0]->tabs)
        {
	        if (tab.rect.Contains(position))
                return false;
        }

        if (controlRects.NonEmpty() && nativeWindow != nullptr && PlatformMisc::GetCurrentPlatform() != PlatformName::Mac)
        {
            if (canBeMinimized && position.x >= controlRects[0].min.x && position.y < 60)
                return false;
            if (canBeMaximized && position.x >= controlRects[1].min.x && position.y < 60)
                return false;
            if (canBeClosed && position.x >= controlRects[2].min.x && position.y < 60)
                return false;
        }

        if (PlatformMisc::GetCurrentPlatform() != PlatformName::Mac && menuItemRects.NonEmpty())
        {
            Rect rect = menuItemRects.Top();
            if (position.y < rect.max.y && position.x < rect.max.x)
                return false;
        }

        if (position.y < 60)
        {
	        return true;
        }
        return false;
    }

} // namespace CE::Widgets
