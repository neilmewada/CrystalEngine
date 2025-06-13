#include "FusionCore.h"

namespace CE
{

    FDockTabWell::FDockTabWell()
    {

    }

    void FDockTabWell::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FHorizontalStack, container)
            .ContentHAlign(HAlign::Left)
            .ContentVAlign(VAlign::Center)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
        );
    }

    void FDockTabWell::UpdateTabWell()
    {
        tabItems.Clear();

        if (Ref<FDockspace> dockspace = owner.Lock())
        {
            for (int i = 0; i < dockspace->tabbedDockWindows.GetSize(); ++i)
            {
                Ref<FDockWindow> dockWindow = dockspace->tabbedDockWindows[i];
                Ref<FDockTabItem> tabItem;

                if (i < container->GetChildCount())
                {
                    tabItem = CastTo<FDockTabItem>(container->GetChild(i).Get());
                }
                else
                {
                    container->AddChild(
                        FAssignNew(FDockTabItem, tabItem)
                    );
                }

                tabItem->owner = this;
                tabItem->isActive = (tabItem == dockspace->selectedTab);

                tabItem->Title(dockWindow->Title());

                tabItems.Add(tabItem);
            }

            // Remove extra tab items
            while (dockspace->tabbedDockWindows.GetSize() < container->GetChildCount())
            {
                FWidget* lastChild = container->GetChild(dockspace->tabbedDockWindows.GetSize()).Get();
                container->RemoveChild(lastChild);
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

    void FDockTabWell::SetActiveTab(Ref<FDockTabItem> tabItem)
    {
        if (Ref<FDockspace> dockspace = owner.Lock())
        {
            dockspace->SetActiveTab(tabItem);
        }
    }

    void FDockTabWell::ApplyStyle()
    {
        Super::ApplyStyle();

        if (Ref<FDockspace> dockspace = owner.Lock())
        {
            dockspace->ApplyStyle();
        }
    }
}

