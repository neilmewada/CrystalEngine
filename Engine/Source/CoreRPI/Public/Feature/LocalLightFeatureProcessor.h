#pragma once

namespace CE::RPI
{
	namespace Limits
	{
        static constexpr u32 LocalLightTileSize = 32;
        static constexpr u32 MaxLightsPerTile = 128;

        
		constexpr u32 MaxNumTiles = (RHI::Limits::MaxRenderTargetResolution * RHI::Limits::MaxRenderTargetResolution) / (LocalLightTileSize * LocalLightTileSize);
        constexpr u32 LightIndexPoolCapacity = MaxNumTiles * MaxLightsPerTile;
	}

    class LocalLightFeatureProcessor;

    ENUM()
    enum class LocalLightType : u32
    {
	    Point = 0,
        Spot
    };
    ENUM_CLASS(LocalLightType);

    struct alignas(16) LocalLightShaderData
    {
        // xyz = world position, w = range (meters)
        Vec4 worldPosAndRange;

        // xyz = color * intensity (linear RGB), w = (optional: unused or intensity if you prefer)
        Vec4 colorAndIntensity;

        // xyz = world-space spot direction (normalized), w = unused
        Vec4 spotLightDirection;

        // radians (outer cone half-angle for spots). For points, set <= 0
        float spotLightAngle;

        LocalLightType lightType = LocalLightType::Point;
    };

    struct CORERPI_API LocalLightInstance
    {
        // These fields are for shadows
        Matrix4x4 viewMatrix;
        Vec3 viewPosition;
        Vec2i pixelResolution;
        Array<ViewPtr> shadowViews;

        Vec4 direction;
        Vec4 colorAndIntensity;
        Vec3 worldPos;
        float temperature = 0;
        f32 range = 0;
        f32 angle = 0;
        LocalLightType lightType = LocalLightType::Point;

        void Init(LocalLightFeatureProcessor* fp);
        void Deinit(LocalLightFeatureProcessor* fp);

        void UpdateSrgs(int imageIndex);

    private:

        RPI::Scene* scene = nullptr;

    public:

        struct Flags
        {
            bool visible : 1 = true;
            bool initialized : 1 = false;
            bool shadows : 1 = true;
        } flags{};

        friend class LocalLightFeatureProcessor;
    };

    struct LocalLightHandleDescriptor
    {

    };

    static constexpr SIZE_T LocalLightArrayElementsPerPage = 128;

    using LocalLightDynamicArray = PagedDynamicArray<LocalLightInstance, LocalLightArrayElementsPerPage>;
    using LocalLightHandle = LocalLightDynamicArray::Handle;

    CLASS()
    class CORERPI_API LocalLightFeatureProcessor : public LightFeatureProcessor
    {
        CE_CLASS(LocalLightFeatureProcessor, LightFeatureProcessor)
    protected:

        LocalLightFeatureProcessor();

        void OnAfterConstruct() override;

        void OnBeforeDestroy() override;

        void Simulate(const SimulatePacket& packet) override;

        void Render(const RenderPacket& packet) override;
        
    public:

        LocalLightHandle AcquireLight(const LocalLightHandleDescriptor& desc);
        bool ReleaseLight(LocalLightHandle& handle);

    protected:

        using uint = u32;
        using uint2 = TVector2<uint>;

        LocalLightDynamicArray lightInstances;

        static constexpr u32 LightDataBufferInitialSize = 64;
        static constexpr u32 LightIndexPoolBufferInitialSize = 64;
        static constexpr u32 TileHeaderBufferInitialSize = 64;
        static constexpr f32 BufferGrowRatio = 1.3f;

        bool initialized = false;
        DynamicStructuredBuffer<LocalLightShaderData> localLights;

    };
    
} // namespace CE

#include "LocalLightFeatureProcessor.rtti.h"
