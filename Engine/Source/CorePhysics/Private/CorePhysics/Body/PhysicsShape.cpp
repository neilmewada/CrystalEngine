#include "CorePhysicsPrivate.h"

namespace CE
{
    ShapeSettings::ShapeSettings(Ref<PhysicsScene> scene) : ownerScene(scene)
    {
    }

    PhysicsShape::PhysicsShape()
    {

    }

	// - Box Shape -

    Ref<BoxShape> BoxShape::Create(const BoxShapeSettings& settings, Ref<Object> outer)
    {
        if (!outer)
            outer = GetTransient(MODULE_NAME);

		Ref<BoxShape> boxShape = CreateObject<BoxShape>(outer.Get(), "BoxShape");
        boxShape->settings = settings;

        return boxShape;
    }

    JPH::Shape* BoxShape::CreateJoltShape() const
    {
        JPH::PhysicsMaterial* mat = physicsMaterial.IsValid() ? new JPH::PhysicsMaterialImpl(physicsMaterial) : nullptr;
        return new JPH::BoxShape(JPH::Vec3(settings.halfExtents.x, settings.halfExtents.y, settings.halfExtents.z), JPH::cDefaultConvexRadius, mat);
    }

    void BoxShape::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

    }

	// - Sphere Shape -

    Ref<SphereShape> SphereShape::Create(const SphereShapeSettings& settings, Ref<Object> outer)
    {
        if (!outer)
            outer = GetTransient(MODULE_NAME);

		Ref<SphereShape> sphereShape = CreateObject<SphereShape>(outer.Get(), "SphereShape");
        sphereShape->settings = settings;

		return sphereShape;
    }

    JPH::Shape* SphereShape::CreateJoltShape() const
    {
        JPH::PhysicsMaterial* mat = physicsMaterial.IsValid() ? new JPH::PhysicsMaterialImpl(physicsMaterial) : nullptr;
		return new JPH::SphereShape(settings.radius, mat);
    }

    void SphereShape::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

    }

	// - Capsule Shape -

    Ref<CapsuleShape> CapsuleShape::Create(const CapsuleShapeSettings& settings, Ref<Object> outer)
    {
        if (!outer)
            outer = GetTransient(MODULE_NAME);

        Ref<CapsuleShape> capsuleShape = CreateObject<CapsuleShape>(outer.Get(), "SphereShape");
        capsuleShape->settings = settings;

        return capsuleShape;
    }

    JPH::Shape* CapsuleShape::CreateJoltShape() const
    {
        JPH::PhysicsMaterial* mat = physicsMaterial.IsValid() ? new JPH::PhysicsMaterialImpl(physicsMaterial) : nullptr;
		return new JPH::CapsuleShape(settings.halfHeight, settings.radius, mat);
    }

    void CapsuleShape::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        
    }

    // - Cylinder Shape -

    Ref<CylinderShape> CylinderShape::Create(const CylinderShapeSettings& settings, Ref<Object> outer)
    {
        if (!outer)
            outer = GetTransient(MODULE_NAME);

        Ref<CylinderShape> cylinderShape = CreateObject<CylinderShape>(outer.Get(), "CylinderShape");
        cylinderShape->settings = settings;

        return cylinderShape;
    }

    JPH::Shape* CylinderShape::CreateJoltShape() const
    {
        JPH::PhysicsMaterial* mat = physicsMaterial.IsValid() ? new JPH::PhysicsMaterialImpl(physicsMaterial) : nullptr;
    	return new JPH::CylinderShape(settings.halfHeight, settings.radius, JPH::cDefaultConvexRadius, mat);
    }

    void CylinderShape::OnBeginDestroy()
    {
	    PhysicsShape::OnBeginDestroy();
    }

    // - Static Compound Shape -

    Ref<StaticCompoundShape> StaticCompoundShape::Create(const StaticCompoundShapeSettings& settings, Ref<Object> outer)
    {
        if (!outer)
            outer = GetTransient(MODULE_NAME);

        Ref<StaticCompoundShape> compoundShape = CreateObject<StaticCompoundShape>(outer.Get(), "StaticCompoundShape");
        compoundShape->settings = settings;

        return compoundShape;
    }

    JPH::Shape* StaticCompoundShape::CreateJoltShape() const
    {
    	if (Ref<PhysicsScene> physicsScene = settings.ownerScene.Lock())
        {
            JPH::StaticCompoundShapeSettings compoundShapeSettings;

            for (int i = 0; i < settings.shapes.GetSize(); ++i)
            {
                Ref<PhysicsShape> physicsShape = settings.shapes[i];
                Vec3 pos = settings.shapePositions[i];
                Quat rot = settings.shapeRotations[i];
                Vec3 scale = settings.shapeScales[i];
                physicsShape->physicsMaterial = physicsMaterial;

                JPH::Ref<JPH::Shape> joltShape = physicsShape->CreateJoltShape();
                JPH::Shape::ShapeResult result = joltShape->ScaleShape(JPH::Vec3Arg(scale.x, scale.y, scale.z));
                if (result.IsValid())
                {
                    joltShape = result.Get();
                }

                compoundShapeSettings.AddShape(JPH::Vec3Arg(pos.x, pos.y, pos.z), JPH::QuatArg(rot.x, rot.y, rot.z, rot.w), joltShape.GetPtr());
            }

            JPH::Shape::ShapeResult result;
            JPH::StaticCompoundShape* outShape = new JPH::StaticCompoundShape(compoundShapeSettings, *physicsScene->GetJoltTempAllocator(), result);
            if (result.HasError())
            {
                delete outShape;
                return nullptr;
            }
            return outShape;
        }

        return nullptr;
    }

    void StaticCompoundShape::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

    }

} // namespace CE

