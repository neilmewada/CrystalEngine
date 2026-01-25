#include "CorePhysicsPrivate.h"

namespace CE
{


    PhysicsSystem& PhysicsSystem::Get()
    {
	    static PhysicsSystem instance{};
		return instance;
    }

    void PhysicsSystem::Initialize(const PhysicsSystemInitInfo& initInfo)
    {
        for (int i = 0; i < physicsLayers.GetCapacity(); ++i)
        {
			physicsLayers[i] = Name();
			isPhysicsLayerStatic[i] = false;
        }

		RegisterBuiltinLayers();

        JPH::RegisterDefaultAllocator();
        
		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();
    }

    void PhysicsSystem::Shutdown()
    {
		JPH::UnregisterTypes();

		for (int i = 0; i < physicsLayers.GetCapacity(); ++i)
		{
			physicsLayers[i] = Name();
			isPhysicsLayerStatic[i] = false;
		}

		delete JPH::Factory::sInstance; JPH::Factory::sInstance = nullptr;
    }

    void PhysicsSystem::Tick(f32 deltaTime)
    {
		for (Ref<PhysicsScene> physicsScene : physicsScenes)
		{
			physicsScene->Tick(deltaTime);
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

	void PhysicsSystem::RegisterScene(PhysicsScene* physicsScene)
	{
		if (!physicsScene)
			return;

		physicsScenes.Add(physicsScene);
	}

	void PhysicsSystem::DeregisterScene(PhysicsScene* physicsScene)
	{
		if (!physicsScene)
			return;

		physicsScenes.Remove(physicsScene);
	}

	void PhysicsSystem::RegisterBuiltinLayers()
    {
		EnumType* enumType = GetStaticEnum<BuiltinPhysicsLayer>();
		if (!enumType)
			return;

		for (int i = 0; i < enumType->GetConstantsCount(); ++i)
		{
			EnumConstant* constant = enumType->GetConstant(i);
			physicsLayers[(u16)constant->GetValue()] = constant->GetName();
		}
    }


} // namespace CE

