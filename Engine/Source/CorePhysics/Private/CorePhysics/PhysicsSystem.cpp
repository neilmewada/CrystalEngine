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

    struct PhysicsSystem::Impl
    {
        ~Impl()
        {
            delete tempAllocator; tempAllocator = nullptr;
			delete jobSystem; jobSystem = nullptr;
			delete physicsSystem; physicsSystem = nullptr;
        }

        JPH::TempAllocatorImpl* tempAllocator = nullptr;
		JPH::JobSystemThreadPool* jobSystem = nullptr;
		JPH::PhysicsSystem* physicsSystem = nullptr;
        BPLayerInterfaceImpl broadphaseLayerInterface;
	};

    PhysicsSystem& PhysicsSystem::Get()
    {
	    static PhysicsSystem instance{};
		return instance;
    }

    void PhysicsSystem::Initialize(const PhysicsSystemInitInfo& initInfo)
    {
        if (impl)
            return;

        for (int i = 0; i < physicsLayers.GetCapacity(); ++i)
        {
			physicsLayers[i] = Name();
			isPhysicsLayerStatic[i] = false;
        }

		RegisterBuiltinLayers();

        JPH::RegisterDefaultAllocator();
        
		impl = new Impl{};
		JPH::Factory::sInstance = new JPH::Factory();

		impl->tempAllocator = new JPH::TempAllocatorImpl(TempAllocatorSize);
		impl->jobSystem = new JPH::JobSystemThreadPool(MaxPhysicsJobs, MaxPhysicsBarriers, 
            Math::Max<SIZE_T>(1, Thread::GetHardwareConcurrency() - NonPhysicsThreadCount));

		BPLayerInterfaceImpl broadPhaseInterface;
		ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
		ObjectLayerPairFilterImpl objectVsObjectLayerFilter;

		impl->physicsSystem = new JPH::PhysicsSystem();
        impl->physicsSystem->Init(MaxPhysicsBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints, broadPhaseInterface, objectVsBroadPhaseLayerFilter, objectVsObjectLayerFilter);
    }

    void PhysicsSystem::Shutdown()
    {
        if (!impl)
			return;

		for (int i = 0; i < physicsLayers.GetCapacity(); ++i)
		{
			physicsLayers[i] = Name();
			isPhysicsLayerStatic[i] = false;
		}

		delete impl; impl = nullptr;
		delete JPH::Factory::sInstance; JPH::Factory::sInstance = nullptr;
    }

    void PhysicsSystem::Tick(f32 deltaTime)
    {
		if (!impl || !impl->physicsSystem)
			return;

		constexpr int CollisionSteps = 1;

		JPH::EPhysicsUpdateError error = impl->physicsSystem->Update(FixedDeltaTime, CollisionSteps, impl->tempAllocator, impl->jobSystem);

		if (error != JPH::EPhysicsUpdateError::None)
		{
			CE_LOG(Error, All, "Physics Update Error: {}", (int)error);
		}
    }

    Name PhysicsSystem::GetPhysicsLayerName(const PhysicsLayer& layer)
    {
		if (!layer.IsValid())
			return {};
		if ((u16)layer >= physicsLayers.GetCapacity())
			return {};

		return physicsLayers[(u16)layer];
    }

    PhysicsLayer PhysicsSystem::RegisterUserPhysicsLayer(const Name& layerName, bool isStatic)
    {
		// Do not register if it is already registered
	    for (u16 i = 0; i < physicsLayers.GetCapacity(); ++i)
	    {
			if (physicsLayers[i] == layerName)
			{
				return i;
			}
	    }

		for (u16 i = 0; i < physicsLayers.GetCapacity(); ++i)
		{
			if (!physicsLayers[i].IsValid())
			{
				physicsLayers[i] = layerName;
				isPhysicsLayerStatic[i] = isStatic;
				return i;
			}
		}

		return {};
    }

    void PhysicsSystem::DeregisterUserPhysicsLayer(const PhysicsLayer& layer)
    {
		if (!layer.IsValid() || (u16)layer >= physicsLayers.GetCapacity())
			return;

		physicsLayers[(u16)layer] = Name();
    }

    Name PhysicsSystem::GetPhysicsLayerName(const PhysicsLayer& layer) const
    {
		if (!layer.IsValid() || (u16)layer >= physicsLayers.GetCapacity())
			return {};

		return physicsLayers[(u16)layer];
    }

	PhysicsLayer PhysicsSystem::GetPhysicsLayer(const Name& layerName) const
	{
		for (u16 i = 0; i < physicsLayers.GetCapacity(); ++i)
		{
			if (physicsLayers[i] == layerName)
				return {i};
		}
		return {};
	}

	bool PhysicsSystem::IsCollisionEnabled(const PhysicsLayer& layerA, const PhysicsLayer& layerB)
	{
		if (!layerA.IsValid() || !layerB.IsValid())
			return true;

		Vec2i layerPair = { (u16)layerA, (u16)layerB };
		return !(collisionsDisabledByLayerPair[layerPair]);
	}

	void PhysicsSystem::SetCollisionEnabled(const PhysicsLayer& layerA, const PhysicsLayer& layerB, bool enabled)
	{
		if (!layerA.IsValid() || !layerB.IsValid())
			return;

		Vec2i layerPair = { (u16)layerA, (u16)layerB };
		Vec2i layerPair2 = { (u16)layerB, (u16)layerA };

		collisionsDisabledByLayerPair[layerPair] = !enabled;
		collisionsDisabledByLayerPair[layerPair2] = !enabled;
	}

    void PhysicsSystem::RegisterBuiltinLayers()
    {
		EnumType* enumType = GetStaticEnum<BuiltinPhysicsLayer>();
		if (!enumType)
			return;

		isPhysicsLayerStatic[(u16)BuiltinPhysicsLayer::Static] = true;

		for (int i = 0; i < enumType->GetConstantsCount(); ++i)
		{
			EnumConstant* constant = enumType->GetConstant(i);
			physicsLayers[(u16)constant->GetValue()] = constant->GetName();
		}
    }

} // namespace CE

