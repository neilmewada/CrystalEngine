#pragma once

namespace CE
{
	class Scene;
	class ActorComponent;
	class SceneComponent;

	CLASS()
	class ENGINE_API Actor : public Object
	{
		CE_CLASS(Actor, Object)
	public:

		Actor();

		// - Public API -

		SceneComponent* SetRootComponent(SubClassType<SceneComponent> componentType, String name = "");

		void SetRootComponent(SceneComponent* rootComponent);

		template<typename TSceneComponent> requires TIsBaseClassOf<SceneComponent, TSceneComponent>::Value
		TSceneComponent* SetRootComponent(const String& name = "")
		{
			return (TSceneComponent*)SetRootComponent(TSceneComponent::StaticType(), name);
		}

		void AttachActor(Actor* actor);
		void DetachActor(Actor* actor);

		void AttachComponent(ActorComponent* actorComponent);
		void DetachComponent(ActorComponent* actorComponent);

		Actor* GetParentActor() const { return parent; }

		CE::Scene* GetScene() const { return scene; }

		virtual void OnBeginPlay();

		virtual void Tick(f32 delta);

		SceneComponent* GetRootComponent() const { return rootComponent; }

		int GetComponentCount() const { return attachedComponents.GetSize(); }

		ActorComponent* GetComponent(int index) const { return attachedComponents[index]; }

		void SetLocalPosition(const Vec3& localPos)
		{
			if (rootComponent)
				rootComponent->SetLocalPosition(localPos);
		}

		bool IsEnabled() const;

		void SetEnabled(bool enabled);

		bool IsSelfEnabled() const { return isEnabled; }

		int GetChildCount() const { return children.GetSize(); }

		Actor* GetChild(int index) const { return children[index]; }

		int GetIndexOfActor(Actor* actor) const { return children.IndexOf(actor); }

	protected:

		void OnFieldChanged(const Name& fieldName) override;
		void OnFieldEdited(const Name& fieldName) override;

		void OnSubobjectAttached(Object* object) override;
		void OnSubobjectDetached(Object* object) override;

		virtual void OnEnabled();

		virtual void OnDisabled();

	private:

		void SetRootComponentInternal(SceneComponent* rootComponent);

		void AttachComponentInternal(ActorComponent* component);
		void DetachComponentInternal(ActorComponent* component);

	private:

		FIELD()
		SceneComponent* rootComponent = nullptr;

		FIELD()
		Array<ActorComponent*> attachedComponents{};

		FIELD()
		Array<Actor*> children{};

		FIELD()
		Actor* parent = nullptr;

		FIELD(Internal)
		CE::Scene* scene = nullptr;

		FIELD(EditAnywhere, Category = "Actor", DisplayName = "Enabled")
		bool isEnabled = true;

		b8 hasBegunPlaying = false;

		friend class CE::Scene;
		friend class SceneComponent;
	};

} // namespace CE

#include "Actor.rtti.h"
