#pragma once

namespace CE
{
    STRUCT()
    struct COREPHYSICS_API PhysicsBodyInitInfo final
    {
        CE_STRUCT(PhysicsBodyInitInfo)
    public:

		PhysicsBodyInitInfo() = default;

        FIELD()
		Ref<PhysicsShape> shape = nullptr; // The shape of the physics body

        FIELD()
        PhysicsLayer layer = BuiltinPhysicsLayer::Static;

	};

    CLASS()
    class COREPHYSICS_API PhysicsBody : public Object
    {
        CE_CLASS(PhysicsBody, Object)
    protected:

        PhysicsBody();

        void OnBeginDestroy() override;
        
    public:


    private:

        struct Impl;
		Impl* impl = nullptr;
    };
    
} // namespace CE

#include "PhysicsBody.rtti.h"
