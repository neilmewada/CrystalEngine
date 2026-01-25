#include "Engine.h"

namespace CE
{

    PhysicsSubsystem::PhysicsSubsystem()
    {

    }

    f32 PhysicsSubsystem::GetTickPriority() const
    {
        return 0;
    }

    void PhysicsSubsystem::Initialize()
    {
	    Super::Initialize();

        PhysicsSystemInitInfo initInfo{};

        PhysicsSystem::Get().Initialize(initInfo);
    }

    void PhysicsSubsystem::PostInitialize()
    {
	    Super::PostInitialize();

    }

    void PhysicsSubsystem::Tick(float deltaTime)
    {
	    Super::Tick(deltaTime);


    }

    void PhysicsSubsystem::PreShutdown()
    {
	    Super::PreShutdown();

        PhysicsSystem::Get().Shutdown();
    }

    void PhysicsSubsystem::Shutdown()
    {
	    Super::Shutdown();
    }
} // namespace CE

