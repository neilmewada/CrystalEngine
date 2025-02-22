#pragma once

namespace CE
{
	class Actor;
	class Scene;
	class SceneComponent;

	CLASS()
	class ENGINE_API ActorComponent : public Object
	{
		CE_CLASS(ActorComponent, Object)
	public:

		ActorComponent();

		// Public API

		virtual bool IsSceneComponent() const { return false; }

		virtual void OnBeginPlay();

		virtual void Tick(f32 delta);

		Actor* GetActor() const { return owner; }

		CE::Scene* GetScene() const;

		virtual bool IsEnabledInHierarchy() const;

		bool IsEnabled() const { return isEnabled; }

		void SetEnabled(bool enabled);

		bool CanTick() const { return canTick; }

		void SetCanTick(bool canTick) { this->canTick = canTick; }

		bool HasBegunPlaying() const { return hasBegunPlaying; }

	protected:

		void OnFieldChanged(const Name& fieldName) override;
		void OnFieldEdited(const Name& fieldName) override;

		virtual void OnEnabled();

		virtual void OnDisabled();

		FIELD()
		bool canTick = true;

		FIELD()
		Actor* owner = nullptr;

	private:

		FIELD(EditAnywhere, Category = "Component", DisplayName = "Enabled")
		bool isEnabled = true;

		b8 hasBegunPlaying = false;
		b8 onEnabledCalled = false;
		b8 onDisabledCalled = false;

		friend class Actor;
		friend class RendererSubsystem;
	};

} // namespace CE

#include "ActorComponent.rtti.h"
