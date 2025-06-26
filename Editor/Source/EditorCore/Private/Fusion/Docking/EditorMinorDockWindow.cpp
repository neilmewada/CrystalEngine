#include "EditorCore.h"

namespace CE::Editor
{

    EditorMinorDockWindow::EditorMinorDockWindow()
    {
        m_AllowedDockspaces = FDockspaceFilter().WithDockTypeMask(FDockTypeMask::All);
    }

    void EditorMinorDockWindow::Construct()
    {
        Super::Construct();

        Style("EditorMinorDockWindow");
    }

    void EditorMinorDockWindow::SetOwnerEditor(Ref<EditorBase> editor)
    {
		this->ownerEditor = editor;
    }
}

