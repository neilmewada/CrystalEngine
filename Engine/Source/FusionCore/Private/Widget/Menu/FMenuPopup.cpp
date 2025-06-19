#include "FusionCore.h"

namespace CE
{

    FMenuPopup::FMenuPopup()
    {
        m_AutoClose = true;
        m_BlockInteraction = false;

        m_Padding = Vec4(1, 1, 1, 1);
    }

    bool FMenuPopup::FocusParentExistsRecursive(FWidget* parent)
    {
	    bool val = Super::FocusParentExistsRecursive(parent) || (ownerItem != nullptr && ownerItem->subMenu == this && ownerItem->FocusParentExistsRecursive(parent));
        return val;
    }

    void FMenuPopup::QueueDestroyAllItems()
    {
        container->QueueDestroyAllChildren();

        menuItems.Clear();
    }

    void FMenuPopup::DestroyAllItems()
    {
        container->DestroyAllChildren();

        menuItems.Clear();
    }

    void FMenuPopup::HandleEvent(FEvent* event)
    {
        if (event->type == FEventType::FocusChanged)
        {
            FFocusEvent* focusEvent = static_cast<FFocusEvent*>(event);

            if (focusEvent->LostFocus() && AutoClose())
            {
                Ref<FWidget> focusedWidget = focusEvent->focusedWidget;

                if (focusedWidget != nullptr)
                {
                    Ref<FMenuItem> ownerItem = this->ownerItem.Lock();

                    if (ownerItem && !focusedWidget->ParentExistsRecursive(ownerItem.Get()))
                    {
                        ClosePopup();
                    }

                    if (ownerItem == nullptr && focusedWidget != this && !focusedWidget->FocusParentExistsRecursive(this))
                    {
                        ClosePopup();

                        Ref<FPopup> parentPopup = this->GetParentPopup();
                        while (parentPopup)
                        {
                            // Do not close the parent popup if we are focused on that one now.
                            if (parentPopup == focusedWidget)
                                break;

                            if (!focusedWidget->FocusParentExistsRecursive(parentPopup.Get()))
                            {
                                parentPopup->ClosePopup();
                            }

                            parentPopup = parentPopup->GetParentPopup();
                        }
                    }

                    while (ownerItem != nullptr)
                    {
                        FWidget* ownerMenu = ownerItem->menuOwner;
                        if (ownerMenu == nullptr)
                            break;

                        if (ownerMenu->IsOfType<FMenuPopup>())
                        {
                            FMenuPopup* menuOwnerPopup = static_cast<FMenuPopup*>(ownerMenu);

                            if (focusedWidget != menuOwnerPopup && !focusedWidget->ParentExistsRecursive(menuOwnerPopup))
                            {
                                menuOwnerPopup->ClosePopup();
                            }

                            ownerItem = menuOwnerPopup->ownerItem.Lock();
                        }
                        else if (ownerMenu->IsOfType<FMenuBar>())
                        {
                            ownerItem = nullptr;
                        }
                    }
                }
                else
                {
                    ClosePopup();
                }
            }
        }

	    Super::HandleEvent(event);
    }

    void FMenuPopup::Construct()
    {
	    Super::Construct();

        Child(
            FAssignNew(FVerticalStack, container)
            .Padding(Vec4(1, 1, 1, 1) * 1.5f)
        );
    }

    void FMenuPopup::OnPopupClosed()
    {
	    Super::OnPopupClosed();

        if (ownerItem != nullptr)
        {
            ownerItem->isHovered = false;
            if (ownerItem->menuOwner)
                ownerItem->menuOwner->ApplyStyle();
        }

        for (WeakRef<FMenuItem> menuItemRef : menuItems)
        {
            if (Ref<FMenuItem> menuItem = menuItemRef.Lock())
            {
                if (menuItem->subMenu)
                {
                    menuItem->subMenu->ClosePopup();
                }
            }
        }
    }
}

