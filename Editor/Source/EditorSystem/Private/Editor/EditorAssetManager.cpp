#include "EditorSystem.h"

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

} // namespace CE::Editor
