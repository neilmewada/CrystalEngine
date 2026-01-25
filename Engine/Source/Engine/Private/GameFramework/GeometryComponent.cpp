#include "Engine.h"

namespace CE
{

	bool CollisionPrimitives::HasShapes() const
	{
		return GetNumShapes() > 0;
	}

	int CollisionPrimitives::GetNumShapes() const
	{
		return spheres.GetSize() + boxes.GetSize() + capsules.GetSize();
	}

	GeometryComponent::GeometryComponent()
	{
		canTick = true;
	}

	void GeometryComponent::Tick(f32 delta)
	{
		Super::Tick(delta);

		if (Ref<CE::Scene> scene = GetScene())
		{
			if (Ref<PhysicsScene> physicsScene = scene->GetPhysicsScene())
			{
				if (simulatePhysics && physicsScene->IsSimulationEnabled() && physicsBody.IsValid())
				{
					Vec3 pos = physicsBody->GetPosition();
					Quat rot = physicsBody->GetRotation();
					Vec3 euler = rot.ToEulerDegrees();

					SetPosition(pos);
					SetRotation(rot);
				}
			}
		}
	}

	void GeometryComponent::OnAttachedToScene(Ref<CE::Scene> scene)
	{
		Super::OnAttachedToScene(scene);

		
	}

	void GeometryComponent::OnTransformFieldEdited(const Name& fieldName)
	{
		Super::OnTransformFieldEdited(fieldName);

		if (physicsBody.IsValid())
		{
			physicsBody->SetPosition(GetPosition());
		}
	}

	void GeometryComponent::OnBeginPlay()
	{
		Super::OnBeginPlay();

		if (Ref<CE::Scene> scene = GetScene())
		{
			if (Ref<PhysicsScene> physicsScene = scene->GetPhysicsScene())
			{
				if (physicsBody != nullptr && physicsBody->GetOwnerScene() != physicsScene)
				{
					physicsBody->BeginDestroy();
					physicsBody = nullptr;
				}

				if (compoundShape)
				{
					compoundShape->BeginDestroy();
				}

				compoundShape = CreatePhysicsShape();

				if (!compoundShape)
				{
					return;
				}

				PhysicsBodyInitInfo bodyInfo{};
				bodyInfo.ownerScene = physicsScene;
				bodyInfo.layer = BuiltinPhysicsLayer::Default;
				if (simulatePhysics)
					bodyInfo.motionType = isKinematic ? PhysicsMotionType::Kinematic : PhysicsMotionType::Dynamic;
				else
					bodyInfo.motionType = PhysicsMotionType::Static;
				bodyInfo.position = GetPosition();
				bodyInfo.rotation = GetRotation();
				bodyInfo.scale = GetLocalScale();
				bodyInfo.objectName = GetName();
				bodyInfo.shape = compoundShape;

				physicsBody = PhysicsBody::Create(bodyInfo, this);
			}
		}
	}

	Ref<PhysicsShape> GeometryComponent::CreatePhysicsShape()
	{
		if (!primitives.HasShapes())
			return nullptr;

		if (Ref<CE::Scene> scene = GetScene())
		{
			if (Ref<PhysicsScene> physicsScene = scene->GetPhysicsScene())
			{
				if (primitives.GetNumShapes() == 1)
				{
					Ref<PhysicsShape> shape = nullptr;

					if (primitives.boxes.NotEmpty())
					{
						shape = BoxShape::Create(primitives.boxes[0], this);
					}

					if (primitives.spheres.NotEmpty())
					{
						shape = SphereShape::Create(primitives.spheres[0], this);
					}

					if (primitives.capsules.NotEmpty())
					{
						shape = CapsuleShape::Create(primitives.capsules[0], this);
					}

					if (shape.IsValid())
					{
						shape->physicsMaterial = physicsMaterial;
					}

					return shape;
				}

				StaticCompoundShapeSettings compoundShapeSettings{};
				compoundShapeSettings.ownerScene = physicsScene;

				for (const auto& box : primitives.boxes)
				{
					compoundShapeSettings.shapes.Add(BoxShape::Create(box, this));
					compoundShapeSettings.shapePositions.Add(box.position);
					compoundShapeSettings.shapeRotations.Add(Quat::EulerDegrees(box.rotation));
					compoundShapeSettings.shapeScales.Add(box.scale);
				}

				for (const auto& capsule : primitives.capsules)
				{
					compoundShapeSettings.shapes.Add(CapsuleShape::Create(capsule, this));
					compoundShapeSettings.shapePositions.Add(capsule.position);
					compoundShapeSettings.shapeRotations.Add(Quat::EulerDegrees(capsule.rotation));
					compoundShapeSettings.shapeScales.Add(capsule.scale);
				}

				for (const auto& sphere : primitives.spheres)
				{
					compoundShapeSettings.shapes.Add(SphereShape::Create(sphere, this));
					compoundShapeSettings.shapePositions.Add(sphere.position);
					compoundShapeSettings.shapeRotations.Add(Quat::EulerDegrees(sphere.rotation));
					compoundShapeSettings.shapeScales.Add(sphere.scale);
				}

				Ref<StaticCompoundShape> compoundShape = StaticCompoundShape::Create(compoundShapeSettings, this);
				if (compoundShape.IsValid())
				{
					compoundShape->physicsMaterial = physicsMaterial;
				}
				return compoundShape;
			}
		}

		return nullptr;
	}

} // namespace CE
