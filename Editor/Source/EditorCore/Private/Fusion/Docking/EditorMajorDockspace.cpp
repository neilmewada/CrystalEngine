#include "EditorCore.h"

namespace CE::Editor
{

    EditorMajorDockspace::EditorMajorDockspace()
    {
		detachedDockspaceWindowClass = EditorMajorDockspaceWindow::StaticClass();

        m_DockspaceType = FDockTypeMask::Major;
    }

    void EditorMajorDockspace::Construct()
    {
        Super::Construct();

        container->GetTabWell()->Padding(Vec4(1, 1.25f, 0, 0) * 15);

        OnCreateTabItem([](FDockTabItem& tabItem)
            {
                tabItem.MinWidth(150);
            });
    }
    
}

