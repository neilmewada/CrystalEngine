#include "CorePhysicsPrivate.h"

namespace CE
{
    struct PhysicsBody::Impl
    {
        ~Impl()
        {
            joltShape = nullptr;
            if (!physicsSystem)
                return;

            JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();
            bodyInterface.DestroyBody(joltBody->GetID());
        }

        JPH::PhysicsSystem* physicsSystem = nullptr;
		JPH::Ref<JPH::Shape> joltShape = nullptr; // The Jolt shape instance
        JPH::Body* joltBody = nullptr; // The Jolt body instance
	};

    PhysicsBody::PhysicsBody()
    {
        
    }

    void PhysicsBody::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        if (Ref<PhysicsScene> scene = ownerScene.Lock())
        {
            scene->bodies.Remove(this);
        }

		delete impl; impl = nullptr; // Ensure the implementation is cleaned up
    }

    Ref<PhysicsBody> PhysicsBody::Create(const PhysicsBodyInitInfo& initInfo, Ref<Object> outer)
    {
        if (!initInfo.ownerScene)
        {
            return nullptr;
		}

        if (!outer)
            outer = GetTransient(MODULE_NAME);

		Name objName = initInfo.objectName.IsValid() ? initInfo.objectName : "PhysicsBody";

        JPH::PhysicsSystem* physicsSystem = initInfo.ownerScene->GetJoltPhysicsSystem();
        JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();

        Ref<PhysicsBody> body = CreateObject<PhysicsBody>(outer.Get(), objName.GetString());

        JPH::Shape* joltShape = initInfo.shape->CreateJoltShape();

        JPH::BodyCreationSettings bodySettings(
            joltShape,
            JPH::Vec3(initInfo.position.x, initInfo.position.y, initInfo.position.z),
            JPH::Quat(initInfo.rotation.x, initInfo.rotation.y, initInfo.rotation.z, initInfo.rotation.w),
            (JPH::EMotionType)initInfo.motionType,
            (u16)initInfo.layer
		);

        joltShape->ScaleShape(JPH::Vec3Arg(initInfo.scale.x, initInfo.scale.y, initInfo.scale.z));

        body->ownerScene = initInfo.ownerScene;

		body->impl = new Impl();
		body->impl->joltShape = joltShape;
        body->impl->joltBody = bodyInterface.CreateBody(bodySettings);

        initInfo.ownerScene->AddBody(body);

        return body;
    }

    JPH::Body* PhysicsBody::GetJoltBody()
    {
    	return impl->joltBody;
    }

} // namespace CE

