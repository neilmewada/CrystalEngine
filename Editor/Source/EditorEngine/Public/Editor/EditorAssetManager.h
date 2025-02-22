#pragma once

namespace CE::Editor
{
	class AssetImporter;

    CLASS()
	class EDITORENGINE_API EditorAssetManager : public AssetManager, IAssetRegistryListener
	{
		CE_CLASS(EditorAssetManager, AssetManager)
	public:

		EditorAssetManager();
		virtual ~EditorAssetManager();

		static EditorAssetManager* Get();

		void Initialize() override;
		void Shutdown() override;

		void Tick(f32 deltaTime) override;

	protected:

		Array<IO::Path> sourceAssetsToImport{};
		Array<Name> recentlyProcessedBundleNames{};

		Array<AssetImporter*> importersInProgress{};

		f32 waitToImportSourceAssets = 0;

		int numAssetsBeingImported = 0;

		PlatformWindow* mainWindow = nullptr;

		Queue<Delegate<void(void)>> mainThreadQueue{};
		Mutex mutex{};
	};

} // namespace CE::Editor

#include "EditorAssetManager.rtti.h"
