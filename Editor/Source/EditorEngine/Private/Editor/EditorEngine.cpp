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
	}

	void EditorEngine::PreShutdown()
	{
		Super::PreShutdown();

		assetProcessor->TerminateAllJobs();
		assetProcessor->BeginDestroy();
		assetProcessor = nullptr;
	}

	void EditorEngine::Tick(f32 deltaTime)
	{
		Super::Tick(deltaTime);

	}


} // namespace CE::Editor
