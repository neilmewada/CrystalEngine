#pragma once

namespace CE
{
	class Actor;
	class ActorComponent;
    class CameraComponent;
    class StaticMeshComponent;
	class SceneComponent;
	class RenderPipeline;
	class SceneSubsystem;
	
	CLASS()
	class ENGINE_API Scene : public Asset
	{
		CE_CLASS(Scene, Asset)
	public:

		Scene();
		virtual ~Scene();
        
		virtual void OnBeginPlay();

		virtual void Tick(f32 delta);

		CameraComponent* GetMainCamera() const { return mainCamera; }

		RPI::Scene* GetRpiScene() const { return rpiScene; }

		void AddActor(Actor* actor);
		void RemoveActor(Actor* actor);

		int GetRootActorCount() const { return actors.GetSize(); }

		Actor* GetRootActor(int index) const { return actors[index]; }

		int GetIndexOfActor(Actor* actor) const;
    
		bool IsEnabled() const { return isEnabled; }

		void SetEnabled(bool set) { isEnabled = set; }
    
		void IterateAllComponents(SubClass<ActorComponent> componentClass, auto callback)
		{
			if (componentClass == nullptr)
				return;

			TypeId componentTypeId = componentClass->GetTypeId();

			if (!componentsByType.KeyExists(componentTypeId))
				return;

			for (auto [uuid, component] : componentsByType[componentTypeId])
			{
				callback(component);
			}
		}

		template<typename TActorComponent> requires TIsBaseClassOf<ActorComponent, TActorComponent>::Value
		void IterateAllComponents(auto callback)
		{
			ClassType* clazz = TActorComponent::StaticType();
			if (clazz == nullptr)
				return;

			TypeId componentTypeId = clazz->GetTypeId();

			if (!componentsByType.KeyExists(componentTypeId))
				return;

			for (auto [uuid, component] : componentsByType[componentTypeId])
			{
				if (component->IsOfType(clazz))
				{
					callback((TActorComponent*)component);
				}
			}
		}

		// - Rendering -

		void AddRenderPipeline(CE::RenderPipeline* renderPipeline, CameraComponent* camera);
		void RemoveRenderPipeline(CE::RenderPipeline* renderPipeline);

		void SetSkyboxCubeMap(TextureCube* cubeMap);

		// - Internal -

		void AddSceneCallbacks(ISceneCallbacks* callbacks);
		void RemoveSceneCallbacks(ISceneCallbacks* callbacks);

	private:

		// - Internal API -

		void RegisterActorComponent(ActorComponent* actorComponent);
		void DeregisterActorComponent(ActorComponent* actorComponent);

		void OnActorChainAttached(Actor* actor);
		void OnActorChainDetached(Actor* actor);

		void RegisterSceneComponent(SceneComponent* sceneComponent);
		void DeregisterSceneComponent(SceneComponent* sceneComponent);

		void OnSceneComponentAttached(SceneComponent* sceneComponent);
		void OnSceneComponentDetached(SceneComponent* sceneComponent);

		void OnCameraComponentAttached(CameraComponent* cameraComponent);
		void OnCameraComponentDetached(CameraComponent* cameraComponent);

		void OnRootComponentSet(SceneComponent* rootComponent, Actor* ownerActor);

	protected:

		FIELD()
		Array<Actor*> actors;
		
		FIELD()
		Array<CE::RenderPipeline*> renderPipelines;

		FIELD()
		SubClass<CE::RenderPipeline> defaultRenderPipeline = nullptr;

		FIELD()
		Ref<TextureCube> skyboxCubeMap = nullptr;
		
	private:

		FIELD()
		bool isEnabled = true;

		b8 isPlaying = false;

		Array<ISceneCallbacks*> sceneCallbacks;

		// - RPI -

		RPI::Scene* rpiScene = nullptr;

		// - Cache -

		HashMap<Uuid, Actor*> actorsByUuid{};
		HashMap<TypeId, HashMap<Uuid, ActorComponent*>> componentsByType{};
		// Includes main camera + other cameras
		Array<CameraComponent*> cameras{};

		CameraComponent* mainCamera = nullptr;

		SceneSubsystem* sceneSubsystem = nullptr;
		RendererSubsystem* rendererSubsystem = nullptr;;

		friend class Actor;
		friend class ActorComponent;
		friend class SceneComponent;
		friend class CameraComponent;
		friend class RendererSubsystem;
		friend class GameViewportSubsystem;
		friend class SceneSubsystem;
		friend class SceneRenderer;
	};
    
} // namespace CE

#include "Scene.rtti.h"
