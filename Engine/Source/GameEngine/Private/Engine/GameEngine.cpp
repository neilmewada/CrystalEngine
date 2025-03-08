#include "GameEngine.h"

namespace CE
{

	GameEngine::GameEngine()
	{
		
	}

	GameEngine::~GameEngine()
	{

	}

    void GameEngine::Initialize()
    {
		Super::Initialize();

		ClassType* assetManagerClass = Super::runtimeAssetManagerClass;
		if (assetManagerClass != nullptr)
		{
			if (!assetManagerClass->IsSubclassOf<AssetManager>())
				assetManagerClass = AssetManager::StaticType();
		}
		else
		{
			assetManagerClass = AssetManager::StaticType();
		}

		assetManager = CreateObject<AssetManager>(this, TEXT("AssetManager"), OF_Transient, assetManagerClass);

		ClassType* gameInstanceClass = Super::gameInstanceClass;
		if (gameInstanceClass == nullptr || !gameInstanceClass->IsSubclassOf<GameInstance>())
			gameInstanceClass = GameInstance::StaticType();

		gameInstance = CreateObject<GameInstance>(this, TEXT("GameInstance"), OF_Transient, gameInstanceClass);
		gameInstances.Add(gameInstance);

		gameInstance->Initialize();

		// Init asset manager & asset registry
		if (assetManager)
			assetManager->Initialize();
    }

	void GameEngine::Shutdown()
	{
		Super::Shutdown();

		gameInstance->Shutdown();

		gameInstances.Remove(gameInstance);
		gameInstance->BeginDestroy();
		gameInstance = nullptr;
	}

} // namespace CE

