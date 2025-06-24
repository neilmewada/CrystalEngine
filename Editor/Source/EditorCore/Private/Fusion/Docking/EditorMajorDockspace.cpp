#include "EditorCore.h"

namespace CE::Editor
{

    EditorMajorDockspace::EditorMajorDockspace()
    {
		detachedDockspaceWindowClass = EditorMajorDockspaceWindow::StaticClass();
    }

    void EditorMajorDockspace::Construct()
    {
        Super::Construct();
        
    }
    
}

