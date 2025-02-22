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

	}

	void EditorEngine::PreShutdown()
	{
		Super::PreShutdown();

		
	}

} // namespace CE::Editor
