#include "FusionCore.h"

namespace CE
{

    FMenuBar::FMenuBar()
    {

    }

    void FMenuBar::OnMenuItemHovered(FMenuItem* hoveredItem)
    {
        bool isSubMenuOpen = false;
        
        for (FMenuItem* menuItem : menuItems)
        {
            if (menuItem->subMenu != nullptr && menuItem->subMenu->IsShown())
            {
                isSubMenuOpen = true;
            }
        }

        if (isSubMenuOpen)
        {
            for (FMenuItem* menuItem : menuItems)
            {
                if (menuItem != hoveredItem && menuItem->subMenu != nullptr && menuItem->subMenu->IsShown())
                {
                    menuItem->subMenu->ClosePopup();
                }
                else if (menuItem == hoveredItem && menuItem->subMenu != nullptr && !menuItem->subMenu->IsShown())
                {
                    menuItem->OpenSubMenu();
                }
            }
        }
    }

    void FMenuBar::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FStackBox, container)
            .Direction(FStackBoxDirection::Horizontal)
            .ContentHAlign(HAlign::Left)
            .ContentVAlign(VAlign::Fill)
        );
    }
    
}

