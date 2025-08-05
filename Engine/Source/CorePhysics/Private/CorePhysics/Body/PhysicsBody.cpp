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

    void PhysicsBody::OnBeforeDestroy()
    {
	    Super::OnBeforeDestroy();

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

        JPH::Ref<JPH::Shape> joltShape = initInfo.shape->CreateJoltShape();

        JPH::Shape::ShapeResult result = joltShape->ScaleShape(JPH::Vec3Arg(initInfo.scale.x, initInfo.scale.y, initInfo.scale.z));
        if (result.IsValid())
        {
            joltShape = result.Get();
        }

        JPH::BodyCreationSettings bodySettings(
            joltShape,
            JPH::Vec3(initInfo.position.x, initInfo.position.y, initInfo.position.z),
            JPH::Quat(initInfo.rotation.x, initInfo.rotation.y, initInfo.rotation.z, initInfo.rotation.w),
            (JPH::EMotionType)initInfo.motionType,
            (u16)initInfo.layer
		);

        bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        bodySettings.mMassPropertiesOverride.mMass = initInfo.mass;

        body->ownerScene = initInfo.ownerScene;

		body->impl = new Impl();
		body->impl->joltShape = joltShape;
        body->impl->joltBody = bodyInterface.CreateBody(bodySettings);
        body->impl->physicsSystem = physicsSystem;

        initInfo.ownerScene->AddBody(body);

        return body;
    }

    JPH::Body* PhysicsBody::GetJoltBody()
    {
    	return impl->joltBody;
    }

    Vec3 PhysicsBody::GetPosition()
    {
        JPH::Vec3 vec3 = impl->joltBody->GetPosition();
        return Vec3(vec3.GetX(), vec3.GetY(), vec3.GetZ());
    }

    Quat PhysicsBody::GetRotation()
    {
        JPH::Quat quat = impl->joltBody->GetRotation();
        return Quat(quat.GetW(), quat.GetX(), quat.GetY(), quat.GetZ());
    }

    void PhysicsBody::SetPosition(Vec3 pos)
    {
        impl->physicsSystem->GetBodyInterface().SetPosition(impl->joltBody->GetID(), JPH::Vec3Arg(pos.x, pos.y, pos.z), JPH::EActivation::Activate);
    }

    void PhysicsBody::SetRotation(Quat rot)
    {
        impl->physicsSystem->GetBodyInterface().SetRotation(impl->joltBody->GetID(), JPH::QuatArg(rot.x, rot.y, rot.z, rot.w), JPH::EActivation::Activate);
    }

    Vec3 PhysicsBody::GetLinearVelocity()
    {
        JPH::Vec3 vec = impl->joltBody->GetLinearVelocity();
        return Vec3(vec.GetX(), vec.GetY(), vec.GetZ());
    }

    void PhysicsBody::SetLinearVelocity(Vec3 velocity)
    {
        impl->joltBody->SetLinearVelocity(JPH::Vec3(velocity.x, velocity.y, velocity.z));
    }

    Vec3 PhysicsBody::GetAngularVelocity()
    {
        JPH::Vec3 vec = impl->joltBody->GetAngularVelocity();
        return Vec3(vec.GetX(), vec.GetY(), vec.GetZ());
    }

    void PhysicsBody::SetAngularVelocity(Vec3 velocity)
    {
        impl->joltBody->SetAngularVelocity(JPH::Vec3(velocity.x, velocity.y, velocity.z));
    }

    void PhysicsBody::ClearImpl()
    {
        impl->joltBody = nullptr;
        impl->joltShape = nullptr;
        impl->physicsSystem = nullptr;
    }
} // namespace CE

