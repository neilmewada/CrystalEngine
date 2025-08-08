#include "EditorCore.h"

namespace CE::Editor
{
	FMenuItem& EditorMenuPopup::NewMenuItem()
	{
        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        return
        FNewOwned(FMenuItem, GetTransient(MODULE_NAME))
        //.Text("New Folder")
        .IconEnabled(true)
        //.Icon(FBrush("/Editor/Assets/Icons/NewFolder"))
        .FontSize(fontSize)
        .ContentPadding(Vec4(5, -2, 5, -2));
	}

    EditorMenuPopup::EditorMenuPopup()
    {
        m_MinWidth = 160;
    }

    void EditorMenuPopup::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

    }
}

