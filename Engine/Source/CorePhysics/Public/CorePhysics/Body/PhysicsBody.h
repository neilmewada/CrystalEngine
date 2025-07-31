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
		Ref<PhysicsShape> shape = nullptr; // The shape of the physics body

        FIELD()
        PhysicsLayer layer = BuiltinPhysicsLayer::Static;

        FIELD()
		PhysicsMotionType motionType = PhysicsMotionType::Static; // The motion type of the physics body

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

        void OnBeginDestroy() override;
        
    public:

        static Ref<PhysicsBody> Create(const PhysicsBodyInitInfo& initInfo, Ref<Object> outer = nullptr);

    private:

        struct Impl;
		Impl* impl = nullptr;
    };
    
} // namespace CE

#include "PhysicsBody.rtti.h"
