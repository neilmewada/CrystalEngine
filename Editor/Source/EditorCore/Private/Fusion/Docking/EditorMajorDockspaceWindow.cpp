#include "EditorCore.h"

namespace CE::Editor
{

    EditorMajorDockspaceWindow::EditorMajorDockspaceWindow()
    {
		m_DockspaceClass = EditorMajorDockspace::StaticClass();
    }

    void EditorMajorDockspaceWindow::Construct()
    {
        Super::Construct();
        
    }
    
}

