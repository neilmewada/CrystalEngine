#pragma once

namespace CE
{
    class PhysicsBody;

    CLASS()
    class COREPHYSICS_API PhysicsScene : public Object
    {
        CE_CLASS(PhysicsScene, Object)
    protected:

        PhysicsScene();

        void OnAfterConstruct() override;

        void OnBeginDestroy() override;
        
    public:

        void Tick(f32 deltaTime);

        JPH::PhysicsSystem* GetJoltPhysicsSystem() const;

        void AddBody(Ref<PhysicsBody> body);
        Ref<PhysicsBody> AddBody(const PhysicsBodyInitInfo& bodyInitInfo);

        void RemoveBody(Ref<PhysicsBody> body);

    private:

        Array<Ref<PhysicsBody>> bodies;

        struct Impl;
        Impl* impl = nullptr;

        friend class PhysicsBody;
    };
    
} // namespace CE

#include "PhysicsScene.rtti.h"
