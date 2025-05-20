#include "FusionCore.h"

namespace CE
{

    FMenuItem::FMenuItem()
    {

    }

    bool FMenuItem::IsActive() const
    {
        return subMenu != nullptr && subMenu->IsShown();
    }

    void FMenuItem::OpenSubMenu()
    {
        if (!subMenu)
            return;

        if (menuOwner == nullptr || menuOwner->IsOfType<FMenuItem>())
        {
            GetContext()->PushLocalPopup(subMenu, globalPosition + Vec2(computedSize.x, 0));

            if (menuOwner)
                menuOwner->ApplyStyle();
        }
        else if (menuOwner != nullptr && menuOwner->IsOfType<FMenuBar>())
        {
            Vec2 offset = Vec2(0, computedSize.y);

            GetContext()->PushLocalPopup(subMenu, globalPosition + offset);

            if (menuOwner)
                menuOwner->ApplyStyle();
        }
    }

    void FMenuItem::Construct()
    {
	    Super::Construct();

        Child(
            FAssignNew(FHorizontalStack, content)
            .ContentVAlign(VAlign::Center)
            (
                FAssignNew(FImage, icon)
                .Width(16)
                .Height(16)
                .Enabled(false)
                .Margin(Vec4(0, 0, 10, 0)),

                FAssignNew(FLabel, label)
                .Text("")
                .FontSize(9)
            )
        );
    }

    void FMenuItem::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent() && event->sender == this)
        {
            if (event->type == FEventType::MouseEnter && !isHovered)
            {
                isHovered = true;
                if (menuOwner)
                    menuOwner->ApplyStyle();

                if (menuOwner && menuOwner->IsOfType<FMenuBar>())
                {
                    FMenuBar* menuBar = static_cast<FMenuBar*>(menuOwner);
                    menuBar->OnMenuItemHovered(this);
                }
                else
                {
                    if (menuOwner && menuOwner->IsOfType<FMenuPopup>())
                    {
                        FMenuPopup* menuPopup = static_cast<FMenuPopup*>(menuOwner);

                        for (WeakRef<FMenuItem> menuItemRef : menuPopup->menuItems)
                        {
	                        if (Ref<FMenuItem> menuItem = menuItemRef.Lock())
	                        {
	                            if (menuItem != this && menuItem->subMenu && menuItem->subMenu->IsShown())
	                            {
	                                menuItem->subMenu->ClosePopup();
	                            }
	                        }
                        }
                    }

                    if (subMenu)
                    {
	                    GetContext()->PushLocalPopup(subMenu, globalPosition + Vec2(computedSize.x, 0));
                    }
                }
            }
            else if (event->type == FEventType::MouseLeave && isHovered && (subMenu == nullptr || !subMenu->IsShown()))
            {
                isHovered = false;
                if (menuOwner)
                    menuOwner->ApplyStyle();
            }
            else if (event->type == FEventType::MousePress)
            {
                if (subMenu && !subMenu->IsShown())
                {
                    Vec2 offset = Vec2(0, computedSize.y);

                    GetContext()->PushLocalPopup(subMenu, globalPosition + offset);

                    if (menuOwner)
                        menuOwner->ApplyStyle();
                }

                if (menuOwner && !subMenu)
                {
                    FWidget* curMenuOwner = menuOwner;

                    while (curMenuOwner != nullptr)
                    {
                        if (curMenuOwner->IsOfType<FMenuPopup>())
                        {
                            auto* menuPopup = static_cast<FMenuPopup*>(curMenuOwner);
                            menuPopup->ClosePopup();

                            if (menuPopup->ownerItem != nullptr)
                            {
                                curMenuOwner = menuPopup->ownerItem->menuOwner;
                            }
                            else
                            {
                                curMenuOwner = nullptr;
                            }
                        }
                        else if (curMenuOwner->IsOfType<FMenuBar>())
                        {
                            curMenuOwner = nullptr;
                        }
                    }
                }

                m_OnClick();
            }
        }

	    Super::HandleEvent(event);
    }

    FMenuItem& FMenuItem::SubMenu(FMenuPopup& subMenu)
    {
        this->subMenu = &subMenu;
        subMenu.ownerItem = this;
        return *this;
    }

}

