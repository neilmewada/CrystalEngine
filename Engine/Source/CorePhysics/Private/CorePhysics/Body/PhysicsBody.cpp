#include "CorePhysicsPrivate.h"

namespace CE
{
    struct PhysicsBody::Impl
    {
        ~Impl() = default;

        JPH::Ref<JPH::Body> joltBody; // The Jolt body instance
	};

    PhysicsBody::PhysicsBody()
    {

    }

    void PhysicsBody::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

		delete impl; impl = nullptr; // Ensure the implementation is cleaned up
    }

} // namespace CE

