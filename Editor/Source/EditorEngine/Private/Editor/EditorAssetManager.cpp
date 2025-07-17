#include "EditorEngine.h"

namespace CE::Editor
{
	EditorAssetManager::EditorAssetManager()
	{
		
	}

	EditorAssetManager::~EditorAssetManager()
	{
		
	}

	EditorAssetManager* EditorAssetManager::Get()
	{
		return (EditorAssetManager*)AssetManager::Get();
	}

	void EditorAssetManager::Initialize()
	{
		Super::Initialize();
		
		assetRegistry->listeners.Add(this);
	}

	void EditorAssetManager::Shutdown()
	{
		assetRegistry->listeners.Remove(this);

		Super::Shutdown();
	}

    void EditorAssetManager::Tick(f32 deltaTime)
    {
		Super::Tick(deltaTime);

		if (mainWindow == nullptr)
			mainWindow = PlatformApplication::Get()->GetMainWindow();

		
    }

    void EditorAssetManager::OnAssetCreated(const IO::Path& bundleAbsolutePath)
    {
		assetRegistry->OnAssetImported(bundleAbsolutePath);
    }

    void EditorAssetManager::OnDirectoryCreated(const IO::Path& absolutePath)
    {
		assetRegistry->OnDirectoryCreated(absolutePath);
    }

    void EditorAssetManager::OnDirectoryRenamed(const Name& originalPath, const Name& newName)
    {
		assetRegistry->OnDirectoryRenamed(originalPath, newName);
    }

    void EditorAssetManager::OnAssetRenamed(const Name& originalPath, const IO::Path& newAbsolutePath, const Name& newName)
    {
		assetRegistry->OnAssetRenamed(originalPath, newAbsolutePath, newName);
    }

    void EditorAssetManager::OnDirectoryAndAssetsDeleted(const Array<Name>& paths)
    {
		assetRegistry->OnDirectoryAndAssetsDeleted(paths);
    }

    void EditorAssetManager::OnThumbnailsUpdated(const Array<Name>& assetPaths)
    {
		for (const auto& assetPath : assetPaths)
		{
			Name thumbnailPath = ThumbnailSystem::GetThumbnailPath(assetPath);

			if (loadedImageViewsByPath.KeyExists(thumbnailPath))
			{
				if (RHI::TextureView* view = loadedImageViewsByPath[thumbnailPath])
				{
					RPISystem::Get().QueueDestroy(view);
					loadedImageViewsByPath.Remove(thumbnailPath);

					delete loadedImageSrgsByPath[thumbnailPath];
					loadedImageSrgsByPath.Remove(thumbnailPath);

					if (Ref<Bundle> bundle = loadedTempBundlesByPath[thumbnailPath])
					{
						bundle->BeginDestroy();
						loadedTempBundlesByPath.Remove(thumbnailPath);
					}
				}
			}
		}
    }
} // namespace CE::Editor
