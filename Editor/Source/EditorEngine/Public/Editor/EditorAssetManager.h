#pragma once

namespace CE::Editor
{
	class AssetImporter;

    CLASS()
	class EDITORENGINE_API EditorAssetManager : public AssetManager, public IAssetRegistryListener, public IThumbnailSystemListener
	{
		CE_CLASS(EditorAssetManager, AssetManager)
	public:

		EditorAssetManager();
		virtual ~EditorAssetManager();

		static EditorAssetManager* Get();

		void Initialize() override;
		void Shutdown() override;

		void Tick(f32 deltaTime) override;

		// - Internal API -

    	void OnAssetCreated(const IO::Path& bundleAbsolutePath);

		void OnDirectoryCreated(const IO::Path& absolutePath);

    	void OnDirectoryRenamed(const Name& originalPath, const Name& newName);

    	void OnAssetRenamed(const Name& originalPath, const IO::Path& newAbsolutePath, const Name& newName);

		void OnDirectoryAndAssetsDeleted(const Array<Name>& paths);

	protected:

		void OnThumbnailsUpdated(const Array<Name>& assetPaths) override;

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
