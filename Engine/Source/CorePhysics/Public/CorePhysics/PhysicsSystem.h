#pragma once

namespace CE
{
	struct PhysicsSystemInitInfo
	{
		
	};

    class COREPHYSICS_API PhysicsSystem final
    {
        CE_NO_COPY_MOVE(PhysicsSystem)
    private:

        PhysicsSystem() = default;
        
    public:

        static constexpr float FixedDeltaTime = 1.0f / 60.0f;

        static PhysicsSystem& Get();

		void Initialize(const PhysicsSystemInitInfo& initInfo);

        void Shutdown();

		void Tick(f32 deltaTime);

		Name GetPhysicsLayerName(const PhysicsLayer& layer);

        PhysicsLayer RegisterUserPhysicsLayer(const Name& layerName, bool isStatic = false);
        void DeregisterUserPhysicsLayer(const PhysicsLayer& layer);

        bool IsPhysicsLayerStatic(const PhysicsLayer& layer) const
        {
            if (!layer.IsValid() || (u16)layer >= isPhysicsLayerStatic.GetCapacity())
                return false;
            return isPhysicsLayerStatic[(u16)layer];
		}

		Name GetPhysicsLayerName(const PhysicsLayer& layer) const;
        PhysicsLayer GetPhysicsLayer(const Name& layerName) const;

        bool IsCollisionEnabled(const PhysicsLayer& layerA, const PhysicsLayer& layerB);

        void SetCollisionEnabled(const PhysicsLayer& layerA, const PhysicsLayer& layerB, bool enabled);

        void RegisterScene(PhysicsScene* physicsScene);
		void DeregisterScene(PhysicsScene* physicsScene);

    private:

        void RegisterBuiltinLayers();

        FixedArray<Name, MaxPhysicsLayers> physicsLayers;
        FixedArray<bool, MaxPhysicsLayers> isPhysicsLayerStatic;

        HashMap<Vec2i, bool> collisionsDisabledByLayerPair;

		Array<Ref<PhysicsScene>> physicsScenes;
    };
    
} // namespace CE

