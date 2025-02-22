#include "Engine.h"

namespace CE
{
    SceneSubsystem::SceneSubsystem()
    {
		
    }

    void SceneSubsystem::Initialize()
	{
		Super::Initialize();

		renderer = gEngine->GetSubsystem<RendererSubsystem>();
	}

	void SceneSubsystem::PostInitialize()
	{
		Super::PostInitialize();

		// TODO: Implement multi scene support later
		
		// Create & set an empty scene by default
		//activeScene = CreateObject<CE::Scene>(this, TEXT("EmptyScene"));
	}

	void SceneSubsystem::PreShutdown()
	{
		if (activeScene)
		{
			activeScene->BeginDestroy();
			activeScene = nullptr;
		}

		Super::PreShutdown();
	}

	void SceneSubsystem::Shutdown()
	{

		Super::Shutdown();
	}

	CE::Scene* SceneSubsystem::FindRpiSceneOwner(RPI::Scene* scene)
	{
		if (!scene)
			return nullptr;

		if (activeScene != nullptr && activeScene->GetRpiScene() == scene)
			return activeScene;

		for (CE::Scene* otherScene : otherScenes)
		{
			if (otherScene->GetRpiScene() == scene)
				return otherScene;
		}

		return nullptr;
	}

	void SceneSubsystem::LoadScene(CE::Scene* scene)
	{
		if (activeScene == scene)
			return;

		activeScene = scene;

		if (activeScene)
		{
			for (ISceneSubsystemCallbacks* callbackHandler : callbackHandlers)
			{
				callbackHandler->OnSceneLoaded(activeScene);
			}
		}

		if (activeScene && isPlaying)
		{
			activeScene->OnBeginPlay();
		}
	}

	void SceneSubsystem::AddCallbacks(ISceneSubsystemCallbacks* callbacks)
	{
		callbackHandlers.Add(callbacks);
	}

	void SceneSubsystem::RemoveCallbacks(ISceneSubsystemCallbacks* callbacks)
	{
		callbackHandlers.Remove(callbacks);
	}

	void SceneSubsystem::Tick(f32 deltaTime)
	{
		Super::Tick(deltaTime);

		if (!isPlaying)
		{
			isPlaying = true;

			if (activeScene != nullptr)
				activeScene->OnBeginPlay();
		}
		
		if (activeScene != nullptr)
		{
			activeScene->Tick(deltaTime);
		}
		
		for (CE::Scene* otherScene : otherScenes)
		{
			otherScene->Tick(deltaTime);
		}
	}

	void SceneSubsystem::OnSceneDestroyed(CE::Scene* scene)
	{
		otherScenes.Remove(scene);

		if (scene == activeScene)
		{
			activeScene = nullptr;
		}

		renderer->OnSceneDestroyed(scene);
	}

} // namespace CE
