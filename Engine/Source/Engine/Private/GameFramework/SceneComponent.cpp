#include "Engine.h"

namespace CE
{

    SceneComponent::SceneComponent()
    {
		canTick = true;
    }

    bool SceneComponent::IsEnabledInHierarchy() const
    {
		if (!parentComponent)
			return Super::IsEnabledInHierarchy();

		return Super::IsEnabledInHierarchy() && parentComponent->IsEnabledInHierarchy();
    }

	void SceneComponent::SetupAttachment(SceneComponent* component)
	{
		if (!component || component == this)
			return;

		if (component->ParentComponentExistsRecursive(this))
		{
			CE_LOG(Error, All, "SceneComponent::SetupAttachment called with a scene component that already exists in it's hierarchy");
			return;
		}

		AttachSubobject(component);

		component->parentComponent = this;
		component->owner = this->owner;
		attachedComponents.Add(component);
		
		if (HasBegunPlaying() && !component->HasBegunPlaying())
		{
			component->OnBeginPlay();
		}

		Ref<CE::Scene> scene = GetScene();
		if (scene != nullptr)
		{
			scene->RegisterSceneComponent(component);
			scene->OnSceneComponentAttached(component);

			if (component->IsOfType<CameraComponent>())
			{
				scene->OnCameraComponentAttached((CameraComponent*)component);
			}

			Class* componentClass = component->GetClass();

			while (componentClass->GetTypeId() != TYPEID(Object))
			{
				scene->componentsByType[componentClass->GetTypeId()][component->GetUuid()] = component;

				componentClass = componentClass->GetSuperClass(0);
			}
		}

		SetDirty();
	}

	bool SceneComponent::ParentComponentExistsRecursive(SceneComponent* component)
	{
		if (this == component)
			return true;

		if (parentComponent)
		{
			parentComponent->ParentComponentExistsRecursive(component);
		}
		return false;
	}

	bool SceneComponent::ComponentExistsRecursive(SceneComponent* component)
	{
		for (auto comp : attachedComponents)
		{
			if (comp == component || comp->ComponentExistsRecursive(component))
				return true;
		}
		return false;
	}

	bool SceneComponent::ComponentExists(SceneComponent* component)
	{
		return attachedComponents.Exists([&](SceneComponent* comp) { return comp == component; });
	}

	Vec3 SceneComponent::GetPosition()
	{
		return globalPosition;
	}

	Quat SceneComponent::GetRotation()
	{
		if (!parentComponent)
		{
			return Quat::EulerDegrees(localEulerAngles);
		}

		Vec3 translation;
		Quat parentGlobalRotation;
		Vec3 scale;

    	parentComponent->transform.Decompose(translation, parentGlobalRotation, scale);

		return parentGlobalRotation * Quat::EulerDegrees(localEulerAngles);
	}

	void SceneComponent::SetPosition(Vec3 pos)
	{
		globalPosition = pos;

		if (!parentComponent)
		{
			SetLocalPosition(pos);
			return;
		}

		Matrix4x4 parentTransformInverse = parentComponent->transform.GetInverse();
		Vec3 newLocalPos = parentTransformInverse * Vec4(pos.x, pos.y, pos.z, 1.0f);

		SetLocalPosition(newLocalPos);
	}

	void SceneComponent::SetRotation(Quat worldRotation)
	{
		if (!parentComponent)
		{
			SetLocalEulerAngles(worldRotation.ToEulerDegrees());
			return;
		}

		Quat parentGlobalRot = parentComponent->transform.GetRotation();
		Quat localRot = parentGlobalRot.GetInversed() * worldRotation;

		SetLocalEulerAngles(localRot.ToEulerDegrees());
	}

	void SceneComponent::OnSubobjectDetached(Object* subobject)
	{
		Super::OnSubobjectDetached(subobject);

		if (subobject->IsOfType<SceneComponent>())
		{
			auto detachedComponent = static_cast<SceneComponent*>(subobject);
			if (Ref<CE::Scene> scene = GetScene())
			{
				scene->DeregisterSceneComponent(detachedComponent);
			}
		}
	}

	void SceneComponent::OnFieldChanged(const Name& fieldName)
	{
		Super::OnFieldChanged(fieldName);

		thread_local const HashSet transformFields = { NAMEOF(localPosition), NAMEOF(localEulerAngles), NAMEOF(localScale) };

		if (transformFields.Exists(fieldName))
		{
			SetDirty();
		}
	}

	void SceneComponent::OnFieldEdited(const Name& fieldName)
	{
		Super::OnFieldEdited(fieldName);

		thread_local const HashSet transformFields = { NAMEOF(localPosition), NAMEOF(localEulerAngles), NAMEOF(localScale) };

		if (transformFields.Exists(fieldName))
		{
			OnTransformFieldEdited(fieldName);

			SetDirty();
		}
	}

	void SceneComponent::OnEnabled()
	{
		Super::OnEnabled();

		for (SceneComponent* attachedComponent : attachedComponents)
		{
			attachedComponent->OnEnabled();
		}
	}

	void SceneComponent::OnDisabled()
	{
		Super::OnDisabled();

		for (SceneComponent* attachedComponent : attachedComponents)
		{
			attachedComponent->OnDisabled();
		}
	}

	void SceneComponent::UpdateTransformInternal()
	{
		auto actor = GetActor();

		localTranslationMat[0][3] = localPosition.x;
		localTranslationMat[1][3] = localPosition.y;
		localTranslationMat[2][3] = localPosition.z;

		localRotation = Quat::EulerDegrees(localEulerAngles);
		localRotationMat = localRotation.ToMatrix();

		localScaleMat[0][0] = localScale.x;
		localScaleMat[1][1] = localScale.y;
		localScaleMat[2][2] = localScale.z;

		localTransform = localTranslationMat * localRotationMat * localScaleMat;
		
		if (parentComponent != nullptr)
		{
			transform = parentComponent->transform * localTransform;
		}
		else if (actor != nullptr && actor->parent != nullptr)
		{
			auto parent = actor->parent;
			while (parent != nullptr)
			{
				if (parent->rootComponent != nullptr)
					break;
				if (parent->GetParentActor() == nullptr)
					break;
				parent = parent->GetParentActor();
			}

			if (parent && parent->rootComponent != nullptr)
				transform = parent->rootComponent->transform * localTransform;
			else
				transform = localTransform;
		}
		else
		{
			transform = localTransform;
		}
	}

	void SceneComponent::OnBeginPlay()
	{
		Super::OnBeginPlay();


	}

	void SceneComponent::Tick(f32 delta)
	{
		Super::Tick(delta);
		transformUpdated = false;
        
		if (IsDirty())
		{
			UpdateTransformInternal();

			globalPosition = transform * Vec4(0, 0, 0, 1);

			forwardVector = transform * Vec4(0, 0, 1, 0);
			upwardVector = transform * Vec4(0, 1, 0, 0);
			rightwardVector = transform * Vec4(1, 0, 0, 0);

			Quat::LookRotation2(forwardVector, upwardVector);

			isDirty = false;
			transformUpdated = true;
		}

		for (auto component : attachedComponents)
		{
			if (component->IsEnabled() && component->CanTick())
			{
				component->Tick(delta);
			}
		}
	}

	bool SceneComponent::IsDirty()
	{
		if (isDirty)
			return true;

		Ref<Actor> actor = GetActor();

		if (parentComponent != nullptr && parentComponent->IsDirty())
			return true;

		if (parentComponent == nullptr)
		{
			auto parentActor = actor->GetParentActor();

			if (actor != nullptr && parentActor != nullptr && parentActor->rootComponent != nullptr)
			{
				return parentActor->rootComponent->IsDirty();
			}
			return false;
		}

		return false;
	}

	void SceneComponent::SetDirty()
	{
		isDirty = true;

		for (SceneComponent* attachedComponent : attachedComponents)
		{
			attachedComponent->SetDirty();
		}

		if (owner && owner->rootComponent == this)
		{
			for (Actor* child : owner->children)
			{
				if (child->rootComponent != nullptr)
				{
					child->rootComponent->SetDirty();
				}
			}
		}
	}

} // namespace CE

