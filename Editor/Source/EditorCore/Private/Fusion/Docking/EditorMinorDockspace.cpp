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
        
    }
    
}

