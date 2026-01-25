#pragma once

namespace CE
{

    ENUM()
    enum class PhysicsMotionType
    {
		Static = 0, // The body is static and does not move
        Kinematic,  // The body is kinematic and can be moved by the user
        Dynamic     // The body is dynamic and is affected by physics forces
	};
    ENUM_CLASS(PhysicsMotionType);

    STRUCT()
    struct COREPHYSICS_API PhysicsBodyInitInfo final
    {
        CE_STRUCT(PhysicsBodyInitInfo)
    public:

		PhysicsBodyInitInfo() = default;

        FIELD()
        Name objectName;

        FIELD()
		Ref<PhysicsScene> ownerScene = nullptr; // The scene that owns this physics body

        FIELD()
		Ref<PhysicsShape> shape = nullptr; // The shape of the physics body

        FIELD()
        PhysicsLayer layer = BuiltinPhysicsLayer::Default;

        FIELD()
		PhysicsMotionType motionType = PhysicsMotionType::Dynamic; // The motion type of the physics body

        FIELD()
        float mass = 1.0f;

        FIELD()
        Vec3 position;

        FIELD()
        Quat rotation;

        FIELD()
        Vec3 scale = Vec3(1, 1, 1);
	};

    CLASS()
    class COREPHYSICS_API PhysicsBody : public Object
    {
        CE_CLASS(PhysicsBody, Object)
    protected:

        PhysicsBody();

        void OnBeforeDestroy() override;
        
    public:

        static Ref<PhysicsBody> Create(const PhysicsBodyInitInfo& initInfo, Ref<Object> outer = nullptr);

        JPH::Body* GetJoltBody();

        Vec3 GetPosition();
        Quat GetRotation();

        void SetPosition(Vec3 pos);
        void SetRotation(Quat rot);

        Vec3 GetLinearVelocity();
        void SetLinearVelocity(Vec3 velocity);

        Vec3 GetAngularVelocity();
        void SetAngularVelocity(Vec3 radiansPerSecond);

        Ref<PhysicsScene> GetOwnerScene() { return ownerScene.Lock(); }

    private:

        WeakRef<PhysicsScene> ownerScene = nullptr;

        struct Impl;
		Impl* impl = nullptr;

        void ClearImpl();

        friend class PhysicsScene;
    };
    
} // namespace CE

#include "PhysicsBody.rtti.h"
