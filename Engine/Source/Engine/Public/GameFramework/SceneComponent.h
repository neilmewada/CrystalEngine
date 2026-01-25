#pragma once


namespace CE
{
	ENUM()
	enum class Mobility
	{
		Static,
		Stationary,
		Moveable
	};
	ENUM_CLASS(Mobility);

    CLASS()
	class ENGINE_API SceneComponent : public ActorComponent
	{
		CE_CLASS(SceneComponent, ActorComponent)
	public:
		SceneComponent();

		// - Public API -

		bool IsSceneComponent() const final { return true; }

		bool IsEnabledInHierarchy() const override final;

		/// @brief Add the parameter scene component as a child of receiving scene component.
		/// @param component: Component to add as child.
		void SetupAttachment(SceneComponent* component);

		bool ParentComponentExistsRecursive(SceneComponent* component);
		bool ComponentExistsRecursive(SceneComponent* component);
		bool ComponentExists(SceneComponent* component);

		Vec3 GetPosition();
		Quat GetRotation();

		void SetPosition(Vec3 globalPosition);
		void SetRotation(Quat globalRotation);

		void OnBeginPlay() override;

		void Tick(f32 delta) override;

		const Matrix4x4& GetTransform() const { return transform; }

		const Matrix4x4& GetLocalTransform() const { return localTransform; }

		const Vec3& GetForwardVector() const { return forwardVector; }

		const Vec3& GetUpwardVector() const { return upwardVector; }

		const Vec3& GetRightwardVector() const { return rightwardVector; }

		u32 GetAttachedComponentCount() const { return attachedComponents.GetSize(); }

		SceneComponent* GetAttachedComponent(u32 index) const { return attachedComponents[index]; }

		SceneComponent* GetParentComponent() const { return parentComponent.Get(); }

		virtual void OnAttachedToScene(Ref<CE::Scene> scene) {}

    protected:

		void OnSubobjectDetached(Object* subobject) override;

		void OnFieldChanged(const Name& fieldName) override;

		void OnFieldEdited(const Name& fieldName) override;

		virtual void OnTransformFieldEdited(const Name& fieldName) {}

		bool IsTransformUpdated() const { return transformUpdated; }

		void OnEnabled() override;
		void OnDisabled() override;

	private:

		void UpdateTransformInternal();

		bool IsDirty();

		void SetDirty();

		FIELD(ReadOnly)
		Array<SceneComponent*> attachedComponents{};

		FIELD(ReadOnly)
		WeakRef<SceneComponent> parentComponent = nullptr;

		FIELD(EditAnywhere, Category = "Transform", DisplayName = "Position", CategoryOrder = "-1")
		Vec3 localPosition{};

		FIELD(EditAnywhere, Category = "Transform", DisplayName = "Rotation")
		Vec3 localEulerAngles{};

		FIELD(EditAnywhere, Category = "Transform", DisplayName = "Scale")
		Vec3 localScale = Vec3(1, 1, 1);

		FIELD(EditAnywhere, Category = "Transform")
		Mobility mobility = Mobility::Static;

		Quat localRotation = Quat::EulerDegrees(0, 0, 0);

		FIELD(ReadOnly)
		Vec3 globalPosition{};

		FIELD(ReadOnly)
		Vec3 forwardVector{};

		FIELD(ReadOnly)
		Vec3 upwardVector{};

		FIELD(ReadOnly)
		Vec3 rightwardVector{};

		b8 isDirty = true;

		b8 transformUpdated = false;
        
        Matrix4x4 transform{};
		Matrix4x4 localTransform{};

		Matrix4x4 localTranslationMat = Matrix4x4::Identity();
		Matrix4x4 localRotationMat = Matrix4x4::Identity();
		Matrix4x4 localScaleMat = Matrix4x4::Identity();

    public: // - Accessors -

		CE_PROPERTY(LocalPosition, localPosition);
		CE_PROPERTY(LocalEulerAngles, localEulerAngles);
		CE_PROPERTY(LocalScale, localScale);
		CE_PROPERTY(Mobility, mobility);
        
        friend class CE::Scene;
		friend class Actor;
		friend class ActorComponent;
	};

} // namespace CE

#include "SceneComponent.rtti.h"
