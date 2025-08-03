#include "Engine.h"

namespace CE
{
    SceneSubsystem::SceneSubsystem()
    {
		
    }

    f32 SceneSubsystem::GetTickPriority() const
    {
		return 1;
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

		for (auto sceneRenderer : sceneRenderers)
		{
			if (sceneRenderer->GetScene() != nullptr && sceneRenderer->GetScene()->GetRpiScene() == scene)
			{
				return sceneRenderer->GetScene().Get();
			}
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

	void SceneSubsystem::AddScene(CE::Scene* scene)
	{
    	if (scene == nullptr || otherScenes.Exists(scene))
    		return;

    	otherScenes.Add(scene);
	}

	void SceneSubsystem::EnqueueSceneRenderer(Ref<SceneRenderer> sceneRenderer)
	{
		if (!sceneRenderer.IsValid())
			return;

		sceneRenderers.Add(sceneRenderer);

		renderer->RebuildFrameGraph();
	}

	void SceneSubsystem::AddCallbacks(ISceneSubsystemCallbacks* callbacks)
	{
		callbackHandlers.Add(callbacks);
	}

	void SceneSubsystem::RemoveCallbacks(ISceneSubsystemCallbacks* callbacks)
	{
		callbackHandlers.Remove(callbacks);
	}

	void SceneSubsystem::PlayActiveScene()
	{
		if (isPlaying || !activeScene)
			return;

		isPlaying = true;
		activeScene->OnBeginPlay();
	}

	void SceneSubsystem::Tick(f32 deltaTime)
	{
		Super::Tick(deltaTime);
		
		if (activeScene != nullptr)
		{
			activeScene->Tick(deltaTime);
		}
		
		for (CE::Scene* otherScene : otherScenes)
		{
			otherScene->Tick(deltaTime);
		}

		for (auto sceneRenderer : sceneRenderers)
		{
			if (Ref<CE::Scene> scene = sceneRenderer->GetScene())
			{
				scene->Tick(deltaTime);
			}
		}
	}

	void SceneSubsystem::OnSceneDestroyed(CE::Scene* scene)
	{
		otherScenes.Remove(scene);

		sceneRenderers.RemoveAll([&](const Ref<SceneRenderer>& sceneRenderer) { return sceneRenderer.IsValid() && sceneRenderer->GetScene() == scene; });

		if (scene == activeScene)
		{
			activeScene = nullptr;
		}

		renderer->OnSceneDestroyed(scene);
	}

} // namespace CE
