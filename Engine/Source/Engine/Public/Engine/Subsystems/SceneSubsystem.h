#pragma once

#include "ISceneSubsystemCallbacks.h"

namespace CE
{
	class Scene;
	class RendererSubsystem;

    CLASS()
	class ENGINE_API SceneSubsystem : public EngineSubsystem
	{
		CE_CLASS(SceneSubsystem, EngineSubsystem)
	public:
		SceneSubsystem();

		CE::Scene* GetActiveScene() { return activeScene; }

		const Array<CE::Scene*>& GetOtherScenes() const { return otherScenes; }

		CE::Scene* FindRpiSceneOwner(RPI::Scene* scene);

		void LoadScene(CE::Scene* scene);

    	void AddScene(CE::Scene* scene);

		void EnqueueSceneRenderer(Ref<SceneRenderer> sceneRenderer);

		void AddCallbacks(ISceneSubsystemCallbacks* callbacks);
		void RemoveCallbacks(ISceneSubsystemCallbacks* callbacks);

	protected:

		void Initialize() override;
		void PostInitialize() override;
		void PreShutdown() override;
		void Shutdown() override;

		void Tick(f32 deltaTime) override;

		void OnSceneDestroyed(CE::Scene* scene);

		FIELD()
		CE::Scene* activeScene = nullptr;
		
		FIELD()
		Array<CE::Scene*> otherScenes{};

		FIELD()
		Array<Ref<SceneRenderer>> sceneRenderers;

    private:

		RendererSubsystem* renderer = nullptr;
		Array<ISceneSubsystemCallbacks*> callbackHandlers{};

		bool isPlaying = false;

		friend class RendererSubsystem;
		friend class CE::Scene;
	};

} // namespace CE

#include "SceneSubsystem.rtti.h"
