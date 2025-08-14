#include "Engine.h"

namespace CE
{

	CE::Scene::Scene()
	{
		defaultRenderPipeline = GetStaticClass<MainRenderPipeline>();
		
		if (!IsDefaultInstance())
		{
			rpiScene = new RPI::Scene();
			rpiScene->AddDefaultFeatureProcessors();

			sceneSubsystem = gEngine->GetSubsystem<SceneSubsystem>();
			rendererSubsystem = gEngine->GetSubsystem<RendererSubsystem>();

			physicsScene = CreateObject<PhysicsScene>(this, "PhysicsScene");
		}
	}

	CE::Scene::~Scene()
	{
		if (gEngine && !IsDefaultInstance())
		{
			SceneSubsystem* sceneSubsystem = gEngine->GetSubsystem<SceneSubsystem>();
			if (sceneSubsystem)
			{
				sceneSubsystem->OnSceneDestroyed(this);
			}
		}
		delete rpiScene; rpiScene = nullptr;
	}

	void CE::Scene::OnAfterConstruct()
	{
		Super::OnAfterConstruct();
	}

	void CE::Scene::OnBeginPlay()
	{
		if (isPlaying)
			return;

		isPlaying = true;

		for (Actor* actor : actors)
		{
			actor->OnBeginPlay();
		}

		if (physicsScene)
		{
			physicsScene->SetSimulationEnabled(isPlaying);
		}
	}

	void CE::Scene::Tick(f32 delta)
	{
		if (rpiScene)
		{
			rpiScene->SetName(GetName());
		}

		for (Actor* actor : actors)
		{
			if (!actor->IsSelfEnabled())
				continue;

			actor->Tick(delta);
		}

		for (CameraComponent* camera : cameras)
		{
			if (!camera->IsEnabledInHierarchy())
				continue;

			camera->windowSize = rpiScene->GetPrimaryViewportSize();

			camera->TickCamera();
		}

		for (CE::RenderPipeline* renderPipeline : renderPipelines)
		{
			renderPipeline->Tick();
		}

		if (physicsScene)
		{
			physicsScene->Tick(delta);
		}
	}

	void CE::Scene::AddActor(Actor* actor)
	{
		if (actor == nullptr)
			return;
		if (actorsByUuid.KeyExists(actor->GetUuid()))
			return;
		
		actors.Add(actor);

		OnActorChainAttached(actor);
	}

	void CE::Scene::RemoveActor(Actor* actor)
	{
		if (actor == nullptr)
			return;
		if (!actorsByUuid.KeyExists(actor->GetUuid()))
			return;

		actors.Remove(actor);

		OnActorChainDetached(actor);
	}

	int CE::Scene::GetIndexOfActor(Actor* actor) const
	{
		return actors.IndexOf(actor);
	}

	void CE::Scene::AddRenderPipeline(CE::RenderPipeline* renderPipeline, CameraComponent* camera)
	{
		if (renderPipeline && !renderPipelines.Exists(renderPipeline))
		{
			rpiScene->AddRenderPipeline(renderPipeline->GetRpiRenderPipeline());
			renderPipelines.Add(renderPipeline);
		}
	}
	
	void CE::Scene::RemoveRenderPipeline(CE::RenderPipeline* renderPipeline)
	{
		rpiScene->RemoveRenderPipeline(renderPipeline->GetRpiRenderPipeline());
		renderPipelines.Remove(renderPipeline);
	}

	void CE::Scene::SetSkyboxCubeMap(TextureCube* cubeMap)
	{
		skyboxCubeMap = cubeMap;
		if (rpiScene && skyboxCubeMap)
		{
			TextureCube* diffuseIrradiance = skyboxCubeMap->GetDiffuseConvolution();
			RPI::Texture* diffuseIrradianceRpi = nullptr;
			if (diffuseIrradiance)
				diffuseIrradianceRpi = diffuseIrradiance->GetRpiTexture();

			rpiScene->SetSkyboxCubeMap(skyboxCubeMap->GetRpiTexture(), diffuseIrradianceRpi);
		}
	}

	void CE::Scene::AddSceneCallbacks(ISceneCallbacks* callbacks)
	{
		if (sceneCallbacks.Exists(callbacks))
			return;

		sceneCallbacks.Add(callbacks);
	}

	void CE::Scene::RemoveSceneCallbacks(ISceneCallbacks* callbacks)
	{
		sceneCallbacks.Remove(callbacks);
	}

	void CE::Scene::RegisterActorComponent(ActorComponent* actorComponent)
	{
		auto componentClass = actorComponent->GetClass();
		auto componentUuid = actorComponent->GetUuid();

		while (componentClass->GetTypeId() != TYPEID(Object))
		{
			componentsByType[componentClass->GetTypeId()][componentUuid] = actorComponent;

			componentClass = componentClass->GetSuperClass(0);
		}
	}

	void CE::Scene::DeregisterActorComponent(ActorComponent* actorComponent)
	{
		auto componentClass = actorComponent->GetClass();
		auto componentUuid = actorComponent->GetUuid();

		while (componentClass->GetTypeId() != TYPEID(Object))
		{
			componentsByType[componentClass->GetTypeId()].Remove(componentUuid);

			componentClass = componentClass->GetSuperClass(0);
		}
	}

	void CE::Scene::OnActorChainAttached(Actor* actor)
	{
		if (!actor)
			return;
		
		std::function<void(SceneComponent*)> recursivelyAddSceneComponents = [&](SceneComponent* sceneComponent)
        {
            if (!sceneComponent)
                return;

			OnSceneComponentAttached(sceneComponent);

            if (sceneComponent->IsOfType<CameraComponent>())
			{
				CameraComponent* camera = (CameraComponent*)sceneComponent;
				OnCameraComponentAttached(camera);
			}

			auto componentClass = sceneComponent->GetClass();
			auto componentUuid = sceneComponent->GetUuid();

			while (componentClass->GetTypeId() != TYPEID(Object))
			{
				componentsByType[componentClass->GetTypeId()][componentUuid] = sceneComponent;

				componentClass = componentClass->GetSuperClass(0);
			}
            
            for (auto component : sceneComponent->attachedComponents)
            {
                if (component != nullptr && component != sceneComponent)
                    recursivelyAddSceneComponents(component);
            }
        };

		std::function<void(Actor*)> recursivelyAdd = [&](Actor* add)
        {
            if (!add)
                return;

			add->scene = this;
            
            if (add->rootComponent != nullptr)
                recursivelyAddSceneComponents(add->rootComponent);
            
			actorsByUuid[add->GetUuid()] = add;

			for (ActorComponent* component : add->attachedComponents)
			{
				component->scene = this;

				auto componentClass = component->GetClass();

				while (componentClass->GetTypeId() != TYPEID(Object))
				{
					componentsByType[componentClass->GetTypeId()][component->GetUuid()] = component;

					componentClass = componentClass->GetSuperClass(0);
				}
			}

            for (auto child : add->children)
            {
                if (child != nullptr && child != add)
                    recursivelyAdd(child);
            }
        };

		recursivelyAdd(actor);

		for (ISceneCallbacks* callbacks : sceneCallbacks)
		{
			callbacks->OnSceneHierarchyUpdated(this);
		}

		if (isPlaying && !actor->hasBegunPlaying)
		{
			actor->OnBeginPlay();
		}
	}

	void CE::Scene::OnActorChainDetached(Actor* actor)
	{
		if (!actor)
			return;
        
		std::function<void(SceneComponent*)> recursivelyRemoveSceneComponents = [&](SceneComponent* sceneComponent)
        {
            if (!sceneComponent)
                return;

			if (sceneComponent->scene == this)
			{
				sceneComponent->scene = nullptr;
			}

			OnSceneComponentDetached(sceneComponent);

            if (sceneComponent->IsOfType<CameraComponent>())
			{
				CameraComponent* camera = (CameraComponent*)sceneComponent;
				OnCameraComponentDetached(camera);
			}

			auto componentClass = sceneComponent->GetClass();
			auto componentUuid = sceneComponent->GetUuid();

			while (componentClass->GetTypeId() != TYPEID(Object))
			{
				componentsByType[componentClass->GetTypeId()].Remove(componentUuid);

				componentClass = componentClass->GetSuperClass(0);
			}
            
            for (auto component : sceneComponent->attachedComponents)
            {
                if (component != nullptr && component != sceneComponent)
                    recursivelyRemoveSceneComponents(component);
            }
        };

		std::function<void(Actor*)> recursivelyRemove = [&](Actor* remove)
        {
            if (!remove)
                return;
            
            if (remove->rootComponent != nullptr)
                recursivelyRemoveSceneComponents(remove->rootComponent);
            
			actorsByUuid.Remove(remove->GetUuid());

			for (ActorComponent* component : remove->attachedComponents)
			{
				if (component->scene == this)
				{
					component->scene = nullptr;
				}

				auto componentClass = component->GetClass();
				auto componentUuid = component->GetUuid();

				while (componentClass->GetTypeId() != TYPEID(Object))
				{
					componentsByType[componentClass->GetTypeId()].Remove(componentUuid);

					componentClass = componentClass->GetSuperClass(0);
				}
			}

			remove->scene = nullptr;

            for (auto child : remove->children)
            {
                if (child != nullptr && child != remove)
                    recursivelyRemove(child);
            }
        };
		
		recursivelyRemove(actor);

		for (ISceneCallbacks* callbacks : sceneCallbacks)
		{
			callbacks->OnSceneHierarchyUpdated(this);
		}
	}

	void CE::Scene::RegisterSceneComponent(SceneComponent* sceneComponent)
	{
		if (!sceneComponent)
			return;

		auto componentClass = sceneComponent->GetClass();
		auto componentUuid = sceneComponent->GetUuid();

		if (componentsByType[componentClass->GetTypeId()].KeyExists(componentUuid))
			return;

		while (componentClass->GetTypeId() != TYPEID(Object))
		{
			componentsByType[componentClass->GetTypeId()][sceneComponent->GetUuid()] = sceneComponent;

			componentClass = componentClass->GetSuperClass(0);
		}
	}

	void CE::Scene::DeregisterSceneComponent(SceneComponent* sceneComponent)
	{
		if (!sceneComponent)
			return;

		auto componentClass = sceneComponent->GetClass();
		auto componentUuid = sceneComponent->GetUuid();

		if (!componentsByType[componentClass->GetTypeId()].KeyExists(componentUuid))
			return;

		while (componentClass->GetTypeId() != TYPEID(Object))
		{
			componentsByType[componentClass->GetTypeId()].Remove(sceneComponent->GetUuid());

			componentClass = componentClass->GetSuperClass(0);
		}
	}

	void CE::Scene::OnSceneComponentAttached(SceneComponent* sceneComponent)
	{
		sceneComponent->scene = this;

		if (sceneComponent->IsOfType<DirectionalLightComponent>())
		{
			auto directionalLight = static_cast<DirectionalLightComponent*>(sceneComponent);

			rpiScene->AddView("DirectionalLightShadow", directionalLight->GetRpiView());
		}
		else if (sceneComponent->IsOfType<LocalLightComponent>())
		{
			auto localLight = static_cast<LocalLightComponent*>(sceneComponent);


		}
	}

	void CE::Scene::OnSceneComponentDetached(SceneComponent* sceneComponent)
	{
		if (sceneComponent->IsOfType<DirectionalLightComponent>())
		{
			auto directionalLight = static_cast<DirectionalLightComponent*>(sceneComponent);

			rpiScene->RemoveView("DirectionalLightShadow", directionalLight->GetRpiView());
		}
		else if (sceneComponent->IsOfType<LocalLightComponent>())
		{
			auto localLight = static_cast<LocalLightComponent*>(sceneComponent);


		}
	}

	void CE::Scene::OnCameraComponentAttached(CameraComponent* camera)
	{
		cameras.Add(camera);

		CE::RenderPipeline* renderPipeline = camera->GetRenderPipeline();

		AddRenderPipeline(renderPipeline, camera);

		if (mainCamera == nullptr)
		{
			mainCamera = camera;
			rpiScene->AddView("MainCamera", mainCamera->rpiView);
			camera->cameraType = CameraType::MainCamera;
		}
		else
		{
			rpiScene->AddView("Camera", camera->rpiView);
			camera->cameraType = CameraType::SecondaryCamera;
		}
	}

	void CE::Scene::OnCameraComponentDetached(CameraComponent* camera)
	{
		cameras.Remove(camera);
		RemoveRenderPipeline(camera->GetRenderPipeline());
		if (mainCamera == camera)
		{
			rpiScene->RemoveView("MainCamera", mainCamera->rpiView);
			mainCamera = cameras.IsEmpty() ? nullptr : cameras.Top();
		}
		else
		{
			rpiScene->RemoveView("Camera", camera->rpiView);
		}
	}

	void CE::Scene::OnRootComponentSet(SceneComponent* rootComponent, Actor* ownerActor)
	{
		
	}

} // namespace CE
