#pragma once

namespace CE
{

	STRUCT()
	struct ENGINE_API CollisionPrimitives final
	{
		CE_STRUCT(CollisionPrimitives)
	public:

		CollisionPrimitives() = default;

		FIELD(EditAnywhere)
		Array<SphereShapeSettings> spheres;

		FIELD(EditAnywhere)
		Array<BoxShapeSettings> boxes;

		FIELD(EditAnywhere)
		Array<CapsuleShapeSettings> capsules;

		bool HasShapes() const;

		int GetNumShapes() const;

	};

	CLASS(Abstract)
	class ENGINE_API GeometryComponent : public SceneComponent
	{
		CE_CLASS(GeometryComponent, SceneComponent)
	public:

		GeometryComponent();

	protected:

		void Tick(f32 delta) override;

		void OnAttachedToScene(Ref<CE::Scene> scene) override;

		void OnTransformFieldEdited(const Name& fieldName) override;

		void OnBeginPlay() override;

		Ref<PhysicsShape> CreatePhysicsShape();

	public:

		Ref<PhysicsBody> GetPhysicsBody() const { return physicsBody; }

	private:

		FIELD()
		Array<Ref<PhysicsBody>> collisionBodies;

		FIELD(EditAnywhere, Category = "Collision", StructTypeName = "")
		CollisionPrimitives primitives;

		FIELD(EditAnywhere, Category = "Collision")
		Ref<PhysicsMaterial> physicsMaterial;

		FIELD(EditAnywhere, Category = "Physics")
		bool simulatePhysics = false;

		FIELD(EditAnywhere, Category = "Physics")
		bool isKinematic = false;

		FIELD()
		Array<Ref<PhysicsShape>> physicsShapes;

		FIELD()
		Ref<PhysicsShape> compoundShape;

		FIELD()
		Ref<PhysicsBody> physicsBody = nullptr;

	};
    
} // namespace CE

#include "GeometryComponent.rtti.h"
