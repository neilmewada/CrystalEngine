
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

	class ContactListener : public JPH::ContactListener
	{
	public:

		virtual void OnContactAdded(const JPH::Body& body1,
			const JPH::Body& body2,
			const JPH::ContactManifold& ioManifold,
			JPH::ContactSettings& ioSettings) override
		{
			const auto& subShapeID1 = ioManifold.mSubShapeID1;
			const auto& subShapeID2 = ioManifold.mSubShapeID2;

			const auto* mat1 = static_cast<const JPH::PhysicsMaterialImpl*>(body1.GetShape()->GetMaterial(subShapeID1));
			const auto* mat2 = static_cast<const JPH::PhysicsMaterialImpl*>(body2.GetShape()->GetMaterial(subShapeID2));

			if (!mat1 || !mat2)
				return;

			float friction1 = mat1->dynamicFriction;
			float friction2 = mat2->dynamicFriction;

			// Detect if we should use static friction
			constexpr float tangentialVelocityThreshold = 0.1f; // Like Unity’s “minimum slip speed”

			// Loop over contact points (usually just 1 or 2)
			for (const auto& contact : ioManifold.mRelativeContactPointsOn1)
			{
				// Get world-space contact point
				JPH::Vec3 worldPos1 = body1.GetCenterOfMassTransform() * contact;
				JPH::Vec3 worldPos2 = body2.GetCenterOfMassTransform() * ioManifold.mRelativeContactPointsOn2[&contact - &ioManifold.mRelativeContactPointsOn1[0]];

				// Estimate contact point velocity on each body
				JPH::Vec3 v1 = body1.GetPointVelocity(worldPos1);
				JPH::Vec3 v2 = body2.GetPointVelocity(worldPos2);

				// Tangential relative velocity
				JPH::Vec3 relVel = v1 - v2;

				// Remove the normal component (only keep lateral slip)
				JPH::Vec3 normal = ioManifold.mWorldSpaceNormal;
				JPH::Vec3 lateralVel = relVel - normal * relVel.Dot(normal);

				if (lateralVel.Length() < tangentialVelocityThreshold)
				{
					// Small slip: use static friction
					friction1 = mat1->staticFriction;
					friction2 = mat2->staticFriction;
				}

				break; // Only sample one contact point (optimization)
			}

			ioSettings.mCombinedFriction = JPH::PhysicsMaterialImpl::Combine(friction1, friction2, mat1->frictionCombine);
			ioSettings.mCombinedRestitution = JPH::PhysicsMaterialImpl::Combine(mat1->bounciness, mat2->bounciness, mat1->bouncinessCombine);
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
			delete contactListener; contactListener = nullptr;
			delete tempAllocator; tempAllocator = nullptr;
			delete jobSystem; jobSystem = nullptr;
	    }

        JPH::PhysicsSystem* physicsSystem = nullptr;

		JPH::TempAllocator* tempAllocator = nullptr;
		JPH::JobSystemThreadPool* jobSystem = nullptr;
		BPLayerInterfaceImpl broadPhaseInterface;
		ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
		ObjectLayerPairFilterImpl objectVsObjectLayerFilter;
		ContactListener* contactListener = nullptr;
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
		impl->jobSystem = new JPH::JobSystemThreadPool(MaxPhysicsJobs, MaxPhysicsBarriers, NumPhysicsThreads);

        impl->physicsSystem = new JPH::PhysicsSystem();
		impl->contactListener = new CE::ContactListener();

		impl->physicsSystem->SetContactListener(impl->contactListener);

		JPH::PhysicsMaterial::sDefault = new JPH::PhysicsMaterialImpl(GetMutableDefaults<PhysicsMaterial>());

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
        JPH::EPhysicsUpdateError error = impl->physicsSystem->Update(deltaTime, CollisionSteps, impl->tempAllocator, impl->jobSystem);
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

