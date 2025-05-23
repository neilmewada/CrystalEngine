#include "CrystalEditor.h"

namespace CE::Editor
{
	static CrystalEditorModule* instance = nullptr;

	CrystalEditorModule* CrystalEditorModule::Get()
	{
		return instance;
	}

	void CrystalEditorModule::StartupModule()
	{
		instance = this;

		AssetEditorRegistry::Get()->RegisterEditor(CE::Scene::StaticClass(), SceneEditor::StaticClass());
		AssetEditorRegistry::Get()->RegisterEditor(CE::Material::StaticClass(), MaterialEditor::StaticClass());
		AssetEditorRegistry::Get()->RegisterEditor(CE::ProjectSettings::StaticClass(), ProjectSettingsEditor::StaticClass());

		assetProcessor = CreateObject<AssetProcessor>(GetTransient(MODULE_NAME), "AssetProcessor");
	}

	void CrystalEditorModule::ShutdownModule()
	{
		instance = nullptr;

		assetProcessor->BeginDestroy();
		assetProcessor = nullptr;

		AssetEditorRegistry::Get()->DeregisterEditor(CE::Scene::StaticClass());
		AssetEditorRegistry::Get()->DeregisterEditor(CE::Material::StaticClass());
		AssetEditorRegistry::Get()->DeregisterEditor(CE::ProjectSettings::StaticClass());
	}

}

#include "CrystalEditor.private.h"

CE_IMPLEMENT_MODULE(CrystalEditor, CE::Editor::CrystalEditorModule)
