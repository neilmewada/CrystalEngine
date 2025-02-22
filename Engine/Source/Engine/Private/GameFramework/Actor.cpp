#include "Engine.h"

namespace CE
{

	Actor::Actor()
	{
		rootComponent = nullptr;
	}

	SceneComponent* Actor::SetRootComponent(SubClassType<SceneComponent> componentType, String name)
	{
		if (componentType == nullptr)
			return nullptr;
		if (rootComponent)
			rootComponent->BeginDestroy();
		rootComponent = nullptr;
		
		if (name.IsEmpty())
			name = componentType->GetName().GetLastComponent();
		rootComponent = CreateObject<SceneComponent>(this, name, OF_NoFlags, componentType);

		if (scene)
		{
			scene->OnRootComponentSet(rootComponent, this);
		}

		return rootComponent;
	}

	void Actor::SetRootComponent(SceneComponent* component)
	{
		if (rootComponent == component)
			return;

		if (rootComponent)
		{
			// If current rootComponent's lifecycle is managed by `this` actor, destroy it.
			// Otherwise, just detach the subobject without destroying it.
			if (rootComponent->ParentExistsInChain(this))
			{
				rootComponent->BeginDestroy();
			}
			else
			{
				DetachSubobject(rootComponent);
			}

			rootComponent = nullptr;
		}

		rootComponent = component;

		if (rootComponent) // component can be set to nullptr
		{
			rootComponent->owner = this;
			AttachSubobject(component);

			if (scene)
			{
				scene->OnRootComponentSet(rootComponent, this);
			}
		}
	}

	void Actor::AttachActor(Actor* actor)
	{
		if (!actor)
			return;
		
		children.Add(actor);
		actor->parent = this;
		actor->scene = scene;

		if (actor->parent != nullptr && actor->parent != this)
		{
			actor->parent->DetachActor(actor);
		}

		if (actor->parent != this)
		{
			AttachSubobject(actor);
		}
        
        if (scene != nullptr)
        {
            scene->OnActorChainAttached(actor);
        }
        
        std::function<void(Actor*)> recursivelySetScene = [&](Actor* inActor)
        {
            inActor->scene = scene;
            
            for (auto child : inActor->children)
            {
                recursivelySetScene(child);
            }
        };
        
        recursivelySetScene(actor);
	}

	void Actor::DetachActor(Actor* actor)
	{
		if (!actor)
			return;
		s64 index = children.IndexOf(actor);
		if (index < 0)
			return;
        
		children.RemoveAt(index);
		actor->parent = nullptr;
		actor->scene = nullptr;
        
		DetachSubobject(actor);
        
        if (scene != nullptr)
        {
            scene->OnActorChainDetached(actor);
        }
        
        std::function<void(Actor*)> recursivelyResetScene = [&](Actor* inActor)
        {
            inActor->scene = nullptr;
            
            for (auto child : inActor->children)
            {
                recursivelyResetScene(child);
            }
        };
        
        recursivelyResetScene(actor);
	}

	void Actor::AttachComponent(ActorComponent* actorComponent)
	{
		if (!actorComponent)
			return;

		AttachSubobject(actorComponent);
	}

	void Actor::DetachComponent(ActorComponent* actorComponent)
	{
		if (!actorComponent)
			return;

		DetachSubobject(actorComponent);
	}

	void Actor::AttachComponentInternal(ActorComponent* component)
	{
		if (!component)
			return;
		if (component->IsOfType<SceneComponent>())
		{
			CE_LOG(Error, All, "Cannot attach a scene component to an actor using AttachComponent(). Use SetRootComponent() instead!");
			return;
		}
		if (attachedComponents.Exists(component))
			return;

		attachedComponents.Add(component);
		component->owner = this;

		if (scene)
		{
			scene->RegisterActorComponent(component);
		}
	}

	void Actor::DetachComponentInternal(ActorComponent* component)
	{
		if (!component || !attachedComponents.Exists(component))
			return;

		if (scene)
		{
			scene->DeregisterActorComponent(component);
		}

		attachedComponents.Remove(component);
		component->owner = nullptr;
	}

	void Actor::OnBeginPlay()
	{
		if (rootComponent)
			rootComponent->OnBeginPlay();

		for (auto component : attachedComponents)
		{
			component->OnBeginPlay();
		}

		for (auto child : children)
		{
			child->OnBeginPlay();
		}

		hasBegunPlaying = true;
	}

	void Actor::Tick(f32 delta)
    {
		if (!IsSelfEnabled())
			return;

		if (rootComponent && rootComponent->CanTick() && rootComponent->IsEnabled())
		{
			rootComponent->Tick(delta);
		}

		for (auto component : attachedComponents)
		{
			if (component->CanTick() && component->IsEnabled())
			{
				component->Tick(delta);
			}
		}

		for (auto child : children)
		{
			if (child->IsSelfEnabled())
			{
				child->Tick(delta);
			}
		}
    }

	bool Actor::IsEnabled() const
	{
		if (!parent)
			return isEnabled;

		return isEnabled && parent->IsEnabled();
	}

	void Actor::SetEnabled(bool enabled)
	{
		if (isEnabled == enabled)
			return;

		isEnabled = enabled;

		if (isEnabled)
		{
			OnEnabled();

			if (rootComponent != nullptr)
			{
				rootComponent->OnEnabled();
			}

			for (ActorComponent* attachedComponent : attachedComponents)
			{
				attachedComponent->OnEnabled();
			}

			for (Actor* child : children)
			{
				child->OnEnabled();
			}
		}
		else
		{
			OnDisabled();

			if (rootComponent != nullptr)
			{
				rootComponent->OnDisabled();
			}

			for (ActorComponent* attachedComponent : attachedComponents)
			{
				attachedComponent->OnDisabled();
			}

			for (Actor* child : children)
			{
				child->OnDisabled();
			}
		}
	}

	void Actor::OnFieldChanged(const Name& fieldName)
	{
		Super::OnFieldChanged(fieldName);

		thread_local const Name isEnabledName = NAMEOF(isEnabled);

		if (fieldName == isEnabledName)
		{
			if (IsEnabled())
				OnEnabled();
			else
				OnDisabled();
		}
	}

	void Actor::OnFieldEdited(const Name& fieldName)
	{
		Super::OnFieldEdited(fieldName);

		thread_local const Name isEnabledName = NAMEOF(isEnabled);

		if (fieldName == isEnabledName)
		{
			if (IsEnabled())
				OnEnabled();
			else
				OnDisabled();
		}
	}

	void Actor::OnSubobjectAttached(Object* object)
	{
		Super::OnSubobjectAttached(object);

		if (object->IsOfType<SceneComponent>())
		{
			auto sceneComponent = static_cast<SceneComponent*>(object);

		}
		else if (object->IsOfType<ActorComponent>())
		{
			auto actorComponent = static_cast<ActorComponent*>(object);

			AttachComponentInternal(actorComponent);
		}
	}

	void Actor::OnSubobjectDetached(Object* object)
	{
		Super::OnSubobjectDetached(object);

		if (object->IsOfType<SceneComponent>())
		{
			auto sceneComponent = static_cast<SceneComponent*>(object);

		}
		else if (object->IsOfType<ActorComponent>())
		{
			auto actorComponent = static_cast<ActorComponent*>(object);

			DetachComponentInternal(actorComponent);
		}
	}

	void Actor::OnEnabled()
	{
		if (rootComponent != nullptr && rootComponent->IsEnabledInHierarchy())
		{
			rootComponent->OnEnabled();
		}

		for (ActorComponent* attachedComponent : attachedComponents)
		{
			if (attachedComponent->IsEnabledInHierarchy())
				attachedComponent->OnEnabled();
		}

		for (Actor* child : children)
		{
			if (child->IsEnabled())
				child->OnEnabled();
		}
	}

	void Actor::OnDisabled()
	{
		if (rootComponent != nullptr && !rootComponent->IsEnabledInHierarchy())
		{
			rootComponent->OnDisabled();
		}

		for (ActorComponent* attachedComponent : attachedComponents)
		{
			if (!attachedComponent->IsEnabledInHierarchy())
				attachedComponent->OnDisabled();
		}

		for (Actor* child : children)
		{
			if (!child->IsEnabled())
				child->OnDisabled();
		}
	}

	void Actor::SetRootComponentInternal(SceneComponent* component)
	{
		if (rootComponent == component)
			return;

		if (rootComponent)
		{
			rootComponent = nullptr;
		}

		rootComponent = component;

		if (rootComponent) // rootComponent can be set to nullptr
		{
			rootComponent->owner = this;

			if (scene)
			{
				scene->OnRootComponentSet(rootComponent, this);
			}
		}
	}

} // namespace CE
