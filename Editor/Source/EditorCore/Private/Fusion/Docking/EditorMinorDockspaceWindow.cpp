#include "EditorCore.h"

namespace CE::Editor
{

    EditorMinorDockspaceWindow::EditorMinorDockspaceWindow()
    {
		m_DockspaceClass = EditorMinorDockspace::StaticClass();
    }

    void EditorMinorDockspaceWindow::Construct()
    {
        Super::Construct();
        
    }
    
}

