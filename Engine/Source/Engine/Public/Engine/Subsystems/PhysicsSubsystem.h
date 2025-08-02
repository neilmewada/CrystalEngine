#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API PhysicsSubsystem : public EngineSubsystem
    {
        CE_CLASS(PhysicsSubsystem, EngineSubsystem)
    protected:

        PhysicsSubsystem();
        
    public:

        f32 GetTickPriority() const override;

        void Initialize() override;
        void PostInitialize() override;

        void Tick(float deltaTime) override;

        void PreShutdown() override;
        void Shutdown() override;

    };
    
} // namespace CE

#include "PhysicsSubsystem.rtti.h"
