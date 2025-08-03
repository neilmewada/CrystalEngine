#include "Engine.h"

namespace CE
{
	bool CollisionPrimitives::HasShapes() const
	{
		return spheres.NotEmpty() || boxes.NotEmpty() || capsules.NotEmpty();
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
				if (physicsScene->IsSimulationEnabled() && physicsBody.IsValid())
				{
					// TODO: Apply positions
					Vec3 pos = physicsBody->GetPosition();
					Quat rot = physicsBody->GetRotation();

					SetPosition(pos);
				}
			}
		}
	}

	void GeometryComponent::OnAttachedToScene(Ref<CE::Scene> scene)
	{
		Super::OnAttachedToScene(scene);

		
	}

	void GeometryComponent::UpdatePhysicsBody()
	{

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

				compoundShape = CreateCompoundShape();

				if (!compoundShape)
				{
					return;
				}

				PhysicsBodyInitInfo bodyInfo{};
				bodyInfo.ownerScene = physicsScene;
				bodyInfo.layer = BuiltinPhysicsLayer::Default;
				bodyInfo.motionType = motionType;
				bodyInfo.position = GetPosition();
				bodyInfo.rotation = GetRotation();
				bodyInfo.scale = GetLocalScale();
				bodyInfo.objectName = GetName();
				bodyInfo.shape = compoundShape;

				physicsBody = PhysicsBody::Create(bodyInfo, this);
			}
		}
	}

	Ref<StaticCompoundShape> GeometryComponent::CreateCompoundShape()
	{
		if (!primitives.HasShapes())
			return nullptr;

		if (Ref<CE::Scene> scene = GetScene())
		{
			if (Ref<PhysicsScene> physicsScene = scene->GetPhysicsScene())
			{
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

				return StaticCompoundShape::Create(compoundShapeSettings, this);
			}
		}

		return nullptr;
	}

} // namespace CE
