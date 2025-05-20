#include "EditorCore.h"

namespace CE::Editor
{

    EditorMenuPopup::EditorMenuPopup()
    {
        m_MinWidth = 160;
    }

    void EditorMenuPopup::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

    }
}

