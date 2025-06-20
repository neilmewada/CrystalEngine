#include "FusionCore.h"

namespace CE
{

    FDockTabWell::FDockTabWell()
    {
        
    }

    void FDockTabWell::Construct()
    {
        Super::Construct();

        (*this)
        .ContentHAlign(HAlign::Left)
        .ContentVAlign(VAlign::Center)
        .HAlign(HAlign::Fill)
        .VAlign(VAlign::Fill)
        ;
    }

    void FDockTabWell::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);

    }

    void FDockTabWell::OnItemsRearranged()
    {
        Super::OnItemsRearranged();

        if (Ref<FDockspaceSplitView> dockspaceSplitView = owner.Lock())
        {
            Array<Ref<FDockWindow>> allWindows = dockspaceSplitView->tabbedDockWindows;

            for (int i = 0; i < allWindows.GetSize(); ++i)
            {
                Ref<FDockWindow> dockWindow = allWindows[i];

                int index = children.IndexOf(dockWindow->item);
                if (index >= 0)
                {
                    dockspaceSplitView->tabbedDockWindows[index] = dockWindow;
                }
            }

            tabItems.Clear();

            for (int i = 0; i < children.GetSize(); ++i)
            {
                if (Ref<FDockTabItem> item = CastTo<FDockTabItem>(children[i].Lock()))
                {
                    tabItems.Add(item);
                }
            }
        }
    }

    void FDockTabWell::UpdateTabWell()
    {
        tabItems.Clear();

        if (Ref<FDockspaceSplitView> dockspaceSplitView = owner.Lock())
        {
            for (int i = 0; i < dockspaceSplitView->tabbedDockWindows.GetSize(); ++i)
            {
                Ref<FDockWindow> dockWindow = dockspaceSplitView->tabbedDockWindows[i];
                Ref<FDockTabItem> tabItem;

                if (i < GetChildCount())
                {
                    tabItem = CastTo<FDockTabItem>(GetChild(i).Get());
                }
                else
                {
                    AddChild(
                        FAssignNew(FDockTabItem, tabItem)
                    );
                }

                tabItem->owner = this;
                tabItem->isActive = (tabItem == dockspaceSplitView->selectedTab);
                tabItem->canBeDetached = dockWindow->CanBeUndocked();

                if (!tabItem->isActive)
                {
                    tabItem->Translation(Vec2());
                }

                dockWindow->item = tabItem;

                tabItem->Title(dockWindow->Title());

                tabItems.Add(tabItem);
            }

            // Remove extra tab items
            while (dockspaceSplitView->tabbedDockWindows.GetSize() < GetChildCount())
            {
                FWidget* lastChild = GetChild(dockspaceSplitView->tabbedDockWindows.GetSize()).Get();
                RemoveChild(lastChild);
            }
        }
    }

    int FDockTabWell::GetTabIndex(Ref<FDockTabItem> tabItem)
    {
        return tabItems.IndexOf(tabItem);
    }

    Ref<FDockTabItem> FDockTabWell::GetTabItem(int index)
    {
        return tabItems[index];
    }

    void FDockTabWell::RemoveTabItem(Ref<FDockTabItem> tabItem)
    {
        if (activeItem == tabItem)
        {
            SetActiveItem(nullptr);
		}

        tabItems.Remove(tabItem);
    }

    Ref<FDockspace> FDockTabWell::GetDockspace()
    {
        if (Ref<FDockspace> dockspace = ownerDockspace.Lock())
        {
            return dockspace;
        }

        if (Ref<FDockspaceSplitView> splitView = owner.Lock())
        {
            ownerDockspace = splitView->GetDockspace();
            return ownerDockspace.Lock();
        }

		return nullptr;
    }

    Ref<FDockspaceSplitView> FDockTabWell::GetDockspaceSplitView()
    {
        return owner.Lock();
    }

    void FDockTabWell::SetActiveTab(Ref<FDockTabItem> tabItem)
    {
        if (Ref<FDockspaceSplitView> dockspaceSplitView = GetDockspaceSplitView())
        {
            dockspaceSplitView->SetActiveTab(tabItem);
        }
    }

    void FDockTabWell::ApplyStyle()
    {
        Super::ApplyStyle();

        if (Ref<FDockspaceSplitView> dockspaceSplitView = GetDockspaceSplitView())
        {
            dockspaceSplitView->ApplyStyle();
        }
    }
}

