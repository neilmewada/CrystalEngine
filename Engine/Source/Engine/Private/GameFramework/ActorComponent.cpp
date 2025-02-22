#include "Engine.h"

namespace CE
{

	ActorComponent::ActorComponent()
	{
		canTick = true;
	}

	void ActorComponent::OnBeginPlay()
	{
		hasBegunPlaying = true;
	}

	void ActorComponent::Tick(f32 delta)
	{
		
	}

	CE::Scene* ActorComponent::GetScene() const
	{
		return owner != nullptr ? owner->GetScene() : nullptr;
	}

	bool ActorComponent::IsEnabledInHierarchy() const
	{
		if (!owner)
			return isEnabled;

		return isEnabled && owner->IsEnabled();
	}

	void ActorComponent::SetEnabled(bool enabled)
	{
		if (isEnabled == enabled)
			return;

		isEnabled = enabled;

		thread_local CE::Name name = "isEnabled";
		this->OnFieldChanged(name);

		if (isEnabled)
			OnEnabled();
		else
			OnDisabled();
	}

	void ActorComponent::OnFieldChanged(const Name& fieldName)
	{
		Super::OnFieldChanged(fieldName);

		thread_local const Name isEnabledName = NAMEOF(isEnabled);

		if (fieldName == isEnabledName)
		{
			if (IsEnabledInHierarchy())
				OnEnabled();
			else
				OnDisabled();
		}
	}

	void ActorComponent::OnFieldEdited(const Name& fieldName)
	{
		Super::OnFieldEdited(fieldName);

		thread_local const Name isEnabledName = NAMEOF(isEnabled);

		if (fieldName == isEnabledName)
		{
			if (IsEnabledInHierarchy())
				OnEnabled();
			else
				OnDisabled();
		}
	}

	void ActorComponent::OnEnabled()
	{
		
	}

	void ActorComponent::OnDisabled()
	{

	}

} // namespace CE

