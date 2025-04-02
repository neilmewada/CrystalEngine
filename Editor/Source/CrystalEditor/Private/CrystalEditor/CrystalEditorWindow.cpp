#include "CrystalEditor.h"

namespace CE::Editor
{
    static CrystalEditorWindow* instance = nullptr;

	CrystalEditorWindow* CrystalEditorWindow::Get()
	{
        return instance;
	}

    CrystalEditorWindow::CrystalEditorWindow()
    {

    }

    void CrystalEditorWindow::Construct()
    {
        Super::Construct();

        instance = this;

        Ref<SceneEditor> sceneEditor = nullptr;
        FAssignNew(SceneEditor, sceneEditor);
        AddDockTab(sceneEditor.Get());

        Ref<ProjectSettings> projectSettings = GetSettings<ProjectSettings>();

        projectLabelParent->Enabled(true);
        projectLabel->Text(projectSettings->projectName);

	    if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
	    {
	        logo->Enabled(false);
	    }

        Style("EditorDockspace");
    }

    void CrystalEditorWindow::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        if (this == instance)
        {
            instance = nullptr;
        }
    }
}

