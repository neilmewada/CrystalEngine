#include "CrystalEditor.h"

namespace CE::Editor
{
    static CrystalEditorWindow* instance = nullptr;

	CrystalEditorWindow* CrystalEditorWindow::Get()
	{
        return instance;
	}

	void CrystalEditorWindow::Launch()
	{
		auto mainWindow = PlatformApplication::Get()->GetMainWindow();
		auto rootContext = FusionApplication::Get()->GetRootContext();

		FNativeContext* crystalEditorCtx = FNativeContext::Create(mainWindow, "CrystalEditor", rootContext);
		rootContext->AddChildContext(crystalEditorCtx);

		Ref<CrystalEditorWindow> crystalEditor = nullptr;

		FAssignNewOwned(CrystalEditorWindow, crystalEditor, crystalEditorCtx);
		crystalEditorCtx->SetOwningWidget(crystalEditor.Get());

		mainWindow->SetResizable(true);
		mainWindow->Show();
	}

	CrystalEditorWindow::CrystalEditorWindow()
    {

    }

    void CrystalEditorWindow::Construct()
    {
        Super::Construct();

        instance = this;

        Ref<SceneEditor> sceneEditor = nullptr;

        FAssignNew(SceneEditor, sceneEditor)
		.HAlign(HAlign::Fill)
		.VAlign(VAlign::Fill);

		dockspace->AddDockWindow(sceneEditor);
        //AddDockTab(sceneEditor.Get());

        Ref<ProjectSettings> projectSettings = GetSettings<ProjectSettings>();

        //projectLabelParent->Enabled(true);
        //projectLabel->Text(projectSettings->projectName);

	    if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
	    {
	        //logo->Enabled(false);
	    }

        Style("EditorDockspace");

		Ref<AssetProcessor> assetProcessor = CrystalEditorModule::Get()->GetAssetProcessor();
		assetProcessor->onProgressUpdate.Bind(FUNCTION_BINDING(this, OnAssetProcessorUpdate));

		assetImportProgressPopup = CreateObject<AssetImportProgressPopup>(this, "AssetImportProgressPopup");
    }

    void CrystalEditorWindow::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

		if (!IsDefaultInstance())
		{
			Ref<AssetProcessor> assetProcessor = CrystalEditorModule::Get()->GetAssetProcessor();
			assetProcessor->onProgressUpdate.UnbindAll(this);
		}

        if (this == instance)
        {
            instance = nullptr;
        }
    }

    void CrystalEditorWindow::OnAssetProcessorUpdate(AssetProcessor* assetProcessor)
    {
		if (assetProcessor->IsInProgress())
		{
			if (!progressPopupShown)
			{
				Vec2 windowSize = GetComputedSize();
				Vec2 popupSize = Vec2(650, 100);
				Vec2 popupPos = (windowSize - popupSize) * 0.5f;

				GetContext()->PushLocalPopup(assetImportProgressPopup.Get(), popupPos, popupSize);
			}

			assetImportProgressPopup->UpdateProgress(assetProcessor);
		}
		else
		{
			// Finished
			assetImportProgressPopup->ClosePopup();
		}
    }
}

