#include "Engine.h"

namespace CE
{
	Array<ClassType*> Engine::subsystemClassQueue{};

	Engine::Engine()
	{

	}

	void Engine::PreInit()
	{
		
	}

	void Engine::Initialize()
	{
		for (auto classType : subsystemClassQueue)
		{
			CreateSubsystem(classType);
		}

		subsystemClassQueue.Clear();

		engineSubsystems.Sort([](EngineSubsystem* a, EngineSubsystem* b) -> bool
			{
				if (a == nullptr || b == nullptr)
					return false;
				return a->GetTickPriority() < b->GetTickPriority();
			});

		for (auto subsystem : engineSubsystems)
		{
			subsystem->Initialize();
		}

		if (assetManager)
			assetManager->Initialize();

		isInitialized = true;
	}

	void Engine::PostInitialize()
	{
		engineSubsystems.Sort([](EngineSubsystem* a, EngineSubsystem* b) -> bool
			{
				if (a == nullptr || b == nullptr)
					return false;
				return a->GetTickPriority() < b->GetTickPriority();
			});

		for (auto subsystem : engineSubsystems)
		{
			subsystem->PostInitialize();
		}

		errorShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/Error");

		if (errorShader)
		{
			errorMaterial = CreateObject<CE::Material>(this, "ErrorMaterial");
			errorMaterial->SetShader(errorShader);
		}
	}

	void Engine::PreShutdown()
	{
		RHI::FrameScheduler::Get()->WaitUntilIdle();

		for (auto subsystem : engineSubsystems) // PreShutdown
			subsystem->PreShutdown();

		if (errorMaterial)
		{
			errorMaterial->BeginDestroy(); errorMaterial = nullptr;
		}
		if (errorShader)
		{
			errorShader->BeginDestroy(); errorShader = nullptr;
		}

		if (assetManager)
			assetManager->Shutdown();
	}

	void Engine::Shutdown()
	{
		isInitialized = false;

		// Shutdown Engine Subsystems
		for (auto subsystem : engineSubsystems) // Shutdown
			subsystem->Shutdown();
		for (auto subsystem : engineSubsystems) // Destroy
			subsystem->BeginDestroy();
		engineSubsystems.Clear();
	}

	void Engine::Tick(f32 deltaTime)
	{
		ZoneScoped;

		if (IsEngineRequestingExit())
		{
			return;
		}

		if (assetManager)
			assetManager->Tick(deltaTime);

		engineSubsystems.Sort([](EngineSubsystem* a, EngineSubsystem* b) -> bool
			{
				if (a == nullptr || b == nullptr)
					return false;
				return a->GetTickPriority() < b->GetTickPriority();
			});
		
		for (auto subsystem : engineSubsystems)
		{
			subsystem->Tick(deltaTime);
		}
		
		{
			LockGuard guard{ mainThreadQueueMutex };

			while (!mainThreadQueue.IsEmpty())
			{
				mainThreadQueue.GetFront().InvokeIfValid();
				mainThreadQueue.PopFront();
			}
		}

		for (auto gameInstance : gameInstances)
		{
			if (gameInstance->IsInitialized())
				gameInstance->Tick(deltaTime);
		}
	}

	void Engine::DispatchOnMainThread(const Delegate<void(void)>& action)
	{
		LockGuard guard{ mainThreadQueueMutex };

		mainThreadQueue.PushBack(action);
	}

	GameInstance* Engine::GetGameInstance()
	{
		if (gameInstances.IsEmpty())
			return nullptr;
		return gameInstances.Top();
	}

	EngineSubsystem* Engine::GetSubsystem(ClassType* subsystemClass)
	{
		for (auto subsystem : engineSubsystems)
		{
			if (subsystem->IsOfType(subsystemClass))
			{
				return subsystem;
			}
		}

		return nullptr;
	}

	SceneSubsystem* Engine::GetSceneSubsystem()
	{
		if (this == nullptr)
			return nullptr;

		return GetSubsystem<SceneSubsystem>();
	}

	void Engine::LoadScene(CE::Scene* scene)
	{
		SceneSubsystem* sceneSubsystem = GetSubsystem<SceneSubsystem>();

		sceneSubsystem->LoadScene(scene);
	}

	void Engine::AddScene(CE::Scene* scene)
	{
		SceneSubsystem* sceneSubsystem = GetSubsystem<SceneSubsystem>();

		sceneSubsystem->AddScene(scene);
	}

	void Engine::EnqueueSceneRenderer(Ref<SceneRenderer> sceneRenderer)
	{
		SceneSubsystem* sceneSubsystem = GetSubsystem<SceneSubsystem>();

		sceneSubsystem->EnqueueSceneRenderer(sceneRenderer);
	}

	CE::Scene* Engine::GetActiveScene()
	{
		SceneSubsystem* sceneSubsystem = GetSubsystem<SceneSubsystem>();

		return sceneSubsystem->GetActiveScene();
	}

	void Engine::AddRenderViewport(FGameWindow* viewport)
	{
		RendererSubsystem* rendererSubsystem = GetSubsystem<RendererSubsystem>();

		rendererSubsystem->AddViewport(viewport);
	}

	void Engine::RemoveRenderViewport(FGameWindow* viewport)
	{
		RendererSubsystem* rendererSubsystem = GetSubsystem<RendererSubsystem>();

		rendererSubsystem->RemoveViewport(viewport);
	}

	EngineSubsystem* Engine::CreateSubsystem(ClassType* type)
	{
		if (type == nullptr || type->GetTypeId() == TYPEID(EngineSubsystem) || !type->IsSubclassOf<EngineSubsystem>())
			return nullptr;
		if (!type->CanBeInstantiated())
			return nullptr;

		auto cdi = (EngineSubsystem*)type->GetDefaultInstance();
		if (cdi == nullptr || !cdi->ShouldBeCreated(this))
			return nullptr;

		// Check if subsystem already exists. Only 1 subsystem object allowed per class type.
		int index = engineSubsystems.IndexOf([&](EngineSubsystem* subsystem) -> bool
			{ return subsystem != nullptr && subsystem->GetType()->GetTypeId() == type->GetTypeId(); });
		
		if (index >= 0)
			return engineSubsystems[index];

		String name = type->GetName().GetLastComponent();
		EngineSubsystem* instance = CreateObject<EngineSubsystem>(this, name, OF_NoFlags, type);
		if (isInitialized)
		{
			instance->Initialize();
		}
		engineSubsystems.Add(instance);
		return instance;
	}

} // namespace CE
