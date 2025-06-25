#include "EditorCore.h"

namespace CE::Editor
{

    EditorDockWindow::EditorDockWindow()
    {
        m_AllowedDockspaces = FDockspaceFilter().WithDockTypeMask(FDockTypeMask::All);
    }

    void EditorDockWindow::Construct()
    {
        Super::Construct();

        
    }
    
}

