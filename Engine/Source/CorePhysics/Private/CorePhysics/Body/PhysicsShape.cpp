#include "CorePhysicsPrivate.h"

namespace CE
{

    PhysicsShape::PhysicsShape()
    {

    }

	// - Box Shape -

    struct BoxShape::Impl
    {
        ~Impl() = default;

        JPH::Ref<JPH::BoxShape> joltShape; // The Jolt shape instance
    };

    Ref<BoxShape> BoxShape::Create(const BoxShapeSettings& settings, Ref<Object> outer)
    {
        if (!outer)
            outer = GetTransient(MODULE_NAME);

		Ref<BoxShape> boxShape = CreateObject<BoxShape>(outer.Get(), "BoxShape");
        boxShape->settings = settings;

    	boxShape->impl = new Impl();
		boxShape->impl->joltShape = new JPH::BoxShape(JPH::Vec3(settings.halfExtents.x, settings.halfExtents.y, settings.halfExtents.z));

        return boxShape;
    }

    JPH::Shape* BoxShape::GetJoltShape() const
    {
		return impl->joltShape.GetPtr(); // Return the Jolt shape instance
    }

    JPH::Shape* BoxShape::CreateJoltShape() const
    {
        return new JPH::BoxShape(JPH::Vec3(settings.halfExtents.x, settings.halfExtents.y, settings.halfExtents.z));
    }

    void BoxShape::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        delete impl; impl = nullptr; // Ensure the implementation is cleaned up
    }

	// - Sphere Shape -

    struct SphereShape::Impl
    {
        ~Impl() = default;

    	JPH::Ref<JPH::SphereShape> joltShape; // The Jolt shape instance
	};

    Ref<SphereShape> SphereShape::Create(const SphereShapeSettings& settings, Ref<Object> outer)
    {
        if (!outer)
            outer = GetTransient(MODULE_NAME);

		Ref<SphereShape> sphereShape = CreateObject<SphereShape>(outer.Get(), "SphereShape");
        sphereShape->settings = settings;

    	sphereShape->impl = new Impl();
        sphereShape->impl->joltShape = new JPH::SphereShape(settings.radius);

		return sphereShape;
    }

    JPH::Shape* SphereShape::GetJoltShape() const
    {
		return impl->joltShape.GetPtr(); // Return the Jolt shape instance
    }

    JPH::Shape* SphereShape::CreateJoltShape() const
    {
		return new JPH::SphereShape(settings.radius);
    }

    void SphereShape::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

		delete impl; impl = nullptr; // Ensure the implementation is cleaned up
    }
} // namespace CE

