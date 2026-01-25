#include "EditorCore.h"

namespace CE::Editor
{

    EditorMinorDockspace::EditorMinorDockspace()
    {
		detachedDockspaceWindowClass = EditorMinorDockspaceWindow::StaticClass();

		m_AllowSplitting = true;
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

