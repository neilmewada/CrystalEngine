#include "EditorCore.h"

namespace CE::Editor
{

    EditorMinorDockspace::EditorMinorDockspace()
    {
		detachedDockspaceWindowClass = EditorMinorDockspaceWindow::StaticClass();
    }

    void EditorMinorDockspace::Construct()
    {
        Super::Construct();

        OnCreateTabItem([](FDockTabItem& tabItem)
            {
                tabItem.MinWidth(120);
            });
    }
    
}

