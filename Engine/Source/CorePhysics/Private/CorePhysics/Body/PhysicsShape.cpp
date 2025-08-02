#include "CorePhysicsPrivate.h"

namespace CE
{

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
        return new JPH::BoxShape(JPH::Vec3(settings.halfExtents.x, settings.halfExtents.y, settings.halfExtents.z));
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
		return new JPH::SphereShape(settings.radius);
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
		return new JPH::CapsuleShape(settings.halfHeight, settings.radius);
    }

    void CapsuleShape::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

    }

} // namespace CE

