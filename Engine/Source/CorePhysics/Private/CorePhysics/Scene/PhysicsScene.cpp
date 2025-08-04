#include "CorePhysicsPrivate.h"

namespace CE
{
	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer STATIC(0);
		static constexpr JPH::BroadPhaseLayer DYNAMIC(1);
		static constexpr JPH::uint NUM_LAYERS(2);
	};

	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		using uint = JPH::uint;

		BPLayerInterfaceImpl()
		{
		}

		virtual uint GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			PhysicsLayer layer = inLayer;

			if (PhysicsSystem::Get().IsPhysicsLayerStatic(layer))
			{
				return BroadPhaseLayers::STATIC;
			}

			return BroadPhaseLayers::DYNAMIC;
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
		{
			switch ((JPH::BroadPhaseLayer::Type)inLayer)
			{
			case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::STATIC:	return "STATIC";
			case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::DYNAMIC:		return "DYNAMIC";
			default:													JPH_ASSERT(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:

	};

	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
		{
			PhysicsLayer layer = inLayer1;

			if (PhysicsSystem::Get().IsPhysicsLayerStatic(layer))
			{
				return inLayer2 == BroadPhaseLayers::DYNAMIC;
			}

			return true;
		}
	};

	/// Class that determines if two object layers can collide
	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
		{
			PhysicsLayer layer1 = inObject1;
			PhysicsLayer layer2 = inObject2;

			return PhysicsSystem::Get().IsCollisionEnabled(layer1, layer2);
		}
	};

    struct PhysicsScene::Impl
    {
	    ~Impl()
	    {
	    	JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();
		    JPH::BodyIDVector bodyIDs;
			physicsSystem->GetBodies(bodyIDs);

			for (JPH::BodyID id : bodyIDs)
			{
				bodyInterface.RemoveBody(id);
				bodyInterface.DestroyBody(id);
			}

			delete physicsSystem; physicsSystem = nullptr;
			delete tempAllocator; tempAllocator = nullptr;
			delete jobSystem; jobSystem = nullptr;
	    }

        JPH::PhysicsSystem* physicsSystem = nullptr;

		JPH::TempAllocator* tempAllocator = nullptr;
		JPH::JobSystemThreadPool* jobSystem = nullptr;
		BPLayerInterfaceImpl broadPhaseInterface;
		ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
		ObjectLayerPairFilterImpl objectVsObjectLayerFilter;
    };

    PhysicsScene::PhysicsScene()
    {

    }

    void PhysicsScene::OnAfterConstruct()
    {
	    Super::OnAfterConstruct();

        if (IsDefaultInstance())
            return;

        PhysicsSystem& physicsSystem = PhysicsSystem::Get();

		impl = new Impl();
		impl->tempAllocator = new JPH::TempAllocatorImpl(TempAllocatorSize);
		impl->jobSystem = new JPH::JobSystemThreadPool(MaxPhysicsJobs, MaxPhysicsBarriers, 2);

        impl->physicsSystem = new JPH::PhysicsSystem();

        impl->physicsSystem->Init(MaxPhysicsBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints,
            impl->broadPhaseInterface, 
            impl->objectVsBroadPhaseLayerFilter, 
            impl->objectVsObjectLayerFilter);

        physicsSystem.RegisterScene(this);
    }

    void PhysicsScene::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

		PhysicsSystem::Get().DeregisterScene(this);

		delete impl; impl = nullptr;
    }

    void PhysicsScene::OnBeforeDestroy()
    {
	    Super::OnBeforeDestroy();

		for (Ref<PhysicsBody> physicsBody : bodies)
		{
			physicsBody->ClearImpl();
			physicsBody->BeginDestroy();
		}
    }

    void PhysicsScene::Tick(f32 deltaTime)
    {
        if (!impl || !impl->physicsSystem || !simulationEnabled)
            return;

        constexpr int CollisionSteps = 1;

        // Update the physics system
        JPH::EPhysicsUpdateError error = impl->physicsSystem->Update(PhysicsSystem::FixedDeltaTime, CollisionSteps, impl->tempAllocator, impl->jobSystem);
        if (error != JPH::EPhysicsUpdateError::None)
        {
            CE_LOG(Error, All, "Physics Update Error: {}", (int)error);
        }
    }

    JPH::PhysicsSystem* PhysicsScene::GetJoltPhysicsSystem() const
    {
        return impl->physicsSystem;
    }

    JPH::TempAllocator* PhysicsScene::GetJoltTempAllocator() const
    {
		return impl->tempAllocator;
    }

    void PhysicsScene::AddBody(Ref<PhysicsBody> body)
    {
		if (bodies.Exists(body))
			return;

        JPH::BodyInterface& bodyInterface = impl->physicsSystem->GetBodyInterface();

        bodyInterface.AddBody(body->GetJoltBody()->GetID(), JPH::EActivation::Activate);

        bodies.Add(body);
    }

	Ref<PhysicsBody> PhysicsScene::AddBody(const PhysicsBodyInitInfo& bodyInitInfo)
    {
		Ref<PhysicsBody> body = PhysicsBody::Create(bodyInitInfo, this);
		return body;
    }

    void PhysicsScene::RemoveBody(Ref<PhysicsBody> body)
    {
        JPH::BodyInterface& bodyInterface = impl->physicsSystem->GetBodyInterface();

        bodyInterface.RemoveBody(body->GetJoltBody()->GetID());

        bodies.Remove(body);
    }

	bool PhysicsScene::IsBodyActive(Ref<PhysicsBody> body)
	{
		return impl->physicsSystem->GetBodyInterface().IsActive(body->GetJoltBody()->GetID());
	}

	void PhysicsScene::SetSimulationEnabled(bool enabled)
	{
		simulationEnabled = enabled;
	}

} // namespace CE

