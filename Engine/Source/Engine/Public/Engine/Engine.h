#pragma once

namespace CE
{
	class AssetManager;
	class GameInstance;
	class EngineSubsystem;
	class SceneSubsystem;
	class Scene;

	CLASS(Abstract, Config = Engine)
	class ENGINE_API Engine : public Object
	{
		CE_CLASS(Engine, Object)
	public:
		// - Functions -

		Engine();

		virtual void PreInit();
		virtual void Initialize();
		virtual void PostInitialize();

		virtual void PreShutdown();
		virtual void Shutdown();

		virtual void Tick(f32 deltaTime);

		void DispatchOnMainThread(const Delegate<void(void)>& action);

		virtual GameInstance* GetGameInstance();

		bool IsInitialized() const { return isInitialized; }

		AssetManager* GetAssetManager() const { return assetManager; }

		EngineSubsystem* GetSubsystem(ClassType* subsystemClass);

		SceneSubsystem* GetSceneSubsystem();

		template<typename TSubsystem> requires TIsBaseClassOf<EngineSubsystem, TSubsystem>::Value
		FORCE_INLINE TSubsystem* GetSubsystem()
		{
			return (TSubsystem*)GetSubsystem(TSubsystem::StaticType());
		}

		// - Public API -

		void LoadScene(CE::Scene* scene);

		CE::Scene* GetActiveScene();

		void AddRenderViewport(FGameWindow* viewport);
		void RemoveRenderViewport(FGameWindow* viewport);

		// - Internal API -

		EngineSubsystem* CreateSubsystem(ClassType* subsystemClass);

	protected: 
		// - Fields -

		FIELD(Config)
		SubClass<AssetManager> runtimeAssetManagerClass = nullptr;

		FIELD(Config)
		SubClass<GameInstance> gameInstanceClass = nullptr;

		FIELD()
		AssetManager* assetManager = nullptr;

		FIELD()
		Array<GameInstance*> gameInstances = {};

		FIELD()
		Array<EngineSubsystem*> engineSubsystems{};

		CE::Queue<Delegate<void()>> mainThreadQueue{};
		SharedMutex mainThreadQueueMutex{};

		b8 isInitialized = false;

		static Array<ClassType*> subsystemClassQueue;

		friend class EngineModule;
	};
    
} // namespace CE

#include "Engine.rtti.h"
