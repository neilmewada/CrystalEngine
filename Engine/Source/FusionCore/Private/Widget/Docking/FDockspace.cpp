#include "FusionCore.h"

namespace CE
{
    FDockspaceFilter::Self& FDockspaceFilter::WithDockTypeMask(FDockTypeMask mask)
    {
        this->allowedDockTypes = mask;
        return *this;
    }

    FDockspaceFilter::Self& FDockspaceFilter::WithAllowedDockspaces(const Array<FDockId>& dockspaceIds)
    {
        this->allowedDockspaces = dockspaceIds;
        return *this;
    }

    FDockspace::FDockspace()
    {
        m_AllowDocking = true;
        m_AllowSplitting = false;
        m_DockspaceType = FDockTypeMask::Minor;
        m_DestroyWhenEmpty = false;

        detachedDockspaceWindowClass = FDockspaceWindow::StaticClass();
    }

    void FDockspace::Construct()
    {
        Super::Construct();

        dockId = FDockId::New();

        Child(
            FNew(FVerticalStack)
            .ContentHAlign(HAlign::Fill)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
				FAssignNew(FDockspaceSplitView, container)
                .FillRatio(1.0f)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
            )
        );

        container->ownerDockspace = this;
    }

    void FDockspace::HandleEvent(FEvent* event)
    {
        

	    Super::HandleEvent(event);
    }

    void FDockspace::UpdateTabs()
    {
        container->UpdateTabs();
    }

    void FDockspace::AddDockWindow(Ref<FDockWindow> dockWindow)
    {
        if (!CanBeDocked(dockWindow) || !container->IsSingular())
            return;

        container->AddDockWindow(dockWindow);
    }

    bool FDockspace::CanBeDocked(Ref<FDockWindow> dockWindow)
    {
        if (!dockWindow)
            return false;

        return (dockWindow->AllowedDockspaces().allowedDockTypes & m_DockspaceType) != 0;
    }

    bool FDockspace::CanDetach(Ref<FDockTabItem> dockTabItem)
    {
        return container->CanDetach(dockTabItem);
    }

    void FDockspace::SetTabWellWindowHitTest(bool set)
    {
        if (container->IsSingular())
        {
            container->GetTabWell()->WindowDragHitTest(set);
        }
    }

    void FDockspace::IterateTabWellsRecursively(const Delegate<void(FDockTabWell& tabWell)>& pred)
    {
        std::function<void(Ref<FDockspaceSplitView>)> visitor = [&](Ref<FDockspaceSplitView> splitView)
            {
                if (Ref<FDockTabWell> tabWell = splitView->GetTabWell())
                {
                    pred(*tabWell);
                }

                for (int i = 0; i < splitView->GetChildrenSplitCount(); ++i)
                {
	                if (Ref<FDockspaceSplitView> child = splitView->GetChildrenSplit(i))
	                {
                        visitor(child);
	                }
                }
            };

        visitor(container);
    }

    Ref<FStackBox> FDockspace::GetRootTabWellParent()
    {
        if (!container->IsSingular())
            return nullptr;

        return container->GetTabWellParent();
    }

    Ref<FDockTabWell> FDockspace::GetRootTabWell()
    {
        if (!container->IsSingular())
            return nullptr;

        return container->GetTabWell();
    }

    FDockspace& FDockspace::TabWellOverlayWidget(FWidget& widget)
    {
        if (container->IsSingular())
        {
            container->tabWellOverlay->AddChild(widget);
        }

        return *this;
    }

}

