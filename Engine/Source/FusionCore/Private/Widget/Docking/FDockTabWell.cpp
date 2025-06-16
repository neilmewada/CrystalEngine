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

    void FDockTabWell::UpdateTabWell()
    {
        tabItems.Clear();

        if (Ref<FDockspace> dockspace = owner.Lock())
        {
            for (int i = 0; i < dockspace->tabbedDockWindows.GetSize(); ++i)
            {
                Ref<FDockWindow> dockWindow = dockspace->tabbedDockWindows[i];
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
                tabItem->isActive = (tabItem == dockspace->selectedTab);

                tabItem->Title(dockWindow->Title());

                tabItems.Add(tabItem);
            }

            // Remove extra tab items
            while (dockspace->tabbedDockWindows.GetSize() < GetChildCount())
            {
                FWidget* lastChild = GetChild(dockspace->tabbedDockWindows.GetSize()).Get();
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

