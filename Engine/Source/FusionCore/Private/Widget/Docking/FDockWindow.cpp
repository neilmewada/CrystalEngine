#include "FusionCore.h"

namespace CE
{

    FDockWindow::FDockWindow()
    {
        m_Title = "Untitled";
        m_AllowedDockspaces = FDockspaceFilter().WithDockTypeMask(FDockTypeMask::Major);
        m_CanBeUndocked = true;
    }

    void FDockWindow::Construct()
    {
        Super::Construct();
        
    }

    void FDockWindow::OnFusionPropertyModified(const CE::Name& propertyName)
    {
        Super::OnFusionPropertyModified(propertyName);

        thread_local static CE::Name titleProperty = "Title";

        if (propertyName == titleProperty)
        {
            if (Ref<FDockspace> dockspace = ownerDockspace.Lock())
            {
                dockspace->UpdateTabs();
            }
        }
    }
}

