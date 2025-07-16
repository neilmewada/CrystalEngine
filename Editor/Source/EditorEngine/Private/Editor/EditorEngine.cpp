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

		thumbnailSystem->Initialize();
	}

	void EditorEngine::PreShutdown()
	{
		thumbnailSystem->Shutdown();
		thumbnailSystem->BeginDestroy();
		thumbnailSystem = nullptr;

		assetProcessor->TerminateAllJobs();
		assetProcessor->BeginDestroy();
		assetProcessor = nullptr;

		Super::PreShutdown();
	}

	void EditorEngine::Tick(f32 deltaTime)
	{
		Super::Tick(deltaTime);

		thumbnailSystem->Tick(deltaTime);
	}


} // namespace CE::Editor
