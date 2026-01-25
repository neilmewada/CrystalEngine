#include "EditorEngine.h"

namespace CE::Editor
{
	EditorEngine::EditorEngine()
	{
		assetManager = CreateDefaultSubobject<EditorAssetManager>("EditorAssetManager", OF_Transient);
	}

	EditorEngine::~EditorEngine()
	{
		
	}

	void EditorEngine::PostInitialize()
	{
		Super::PostInitialize();

		assetProcessor = CreateObject<AssetProcessor>(this, "AssetProcessor");
		thumbnailSystem = CreateObject<ThumbnailSystem>(this, "ThumbnailSystem");

		if (gEditorMode != EditorMode::AssetProcessor)
		{
			thumbnailSystem->Initialize();
			thumbnailSystem->AddThumbnailListener(EditorAssetManager::Get());
		}
	}

	void EditorEngine::PreShutdown()
	{
		if (thumbnailSystem)
		{
			thumbnailSystem->RemoveThumbnailListener(EditorAssetManager::Get());
			thumbnailSystem->Shutdown();
			thumbnailSystem->BeginDestroy();
			thumbnailSystem = nullptr;
		}

		assetProcessor->TerminateAllJobs();
		assetProcessor->BeginDestroy();
		assetProcessor = nullptr;

		Super::PreShutdown();
	}

	void EditorEngine::Tick(f32 deltaTime)
	{
		Super::Tick(deltaTime);

		if (thumbnailSystem)
		{
			thumbnailSystem->Tick(deltaTime);
		}
	}


} // namespace CE::Editor
