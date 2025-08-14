#pragma once

namespace CE::RPI
{
	class DirectionalLightFeatureProcessor;

    struct alignas(16) DirectionalLightConstants
    {
        Matrix4x4 lightSpaceMatrix;
        Vec4 direction;
        Vec4 colorAndIntensity;
        float temperature;
		u32 shadow = 0;
    };

	class CORERPI_API DirectionalLightInstance
	{
	public:

		Vec4 direction;
		Matrix4x4 projectionMatrix;
		Matrix4x4 viewMatrix;
		Matrix4x4 viewProjectionMatrix;
		Vec3 viewPosition;
		Vec2i pixelResolution;
		
		Vec4 colorAndIntensity;
		float temperature = 0;
		float shadowDistance = 10;

		ViewPtr shadowView = nullptr;

		void Init(DirectionalLightFeatureProcessor* fp);
		void Deinit(DirectionalLightFeatureProcessor* fp);

		void UpdateSrgs(u32 imageIndex);

	private:

		RPI::Scene* scene = nullptr;

		//RHI::ShaderResourceGroup* viewSrg = nullptr;
		//StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> viewConstantBuffers{};

	public:

		struct Flags
		{
			bool visible : 1 = true;
			bool initialized : 1 = false;
			bool shadows : 1 = true;
		} flags{};

		friend class DirectionalLightFeatureProcessor;
	};

	using DirectionalLightDynamicArray = PagedDynamicArray<DirectionalLightInstance, 16>;
	using DirectionalLightHandle = DirectionalLightDynamicArray::Handle;

	struct DirectionalLightHandleDescriptor
	{
		
	};

    CLASS()
    class CORERPI_API DirectionalLightFeatureProcessor : public LightFeatureProcessor
    {
        CE_CLASS(DirectionalLightFeatureProcessor, LightFeatureProcessor)
    public:

        DirectionalLightFeatureProcessor();

        virtual ~DirectionalLightFeatureProcessor();

		DirectionalLightHandle AcquireLight(const DirectionalLightHandleDescriptor& desc);
		bool ReleaseLight(DirectionalLightHandle& handle);

		void Simulate(const SimulatePacket& packet) override;

		void Render(const RenderPacket& packet) override;

		int GetMaxDirectionalLights();

    protected:

		DirectionalLightDynamicArray lightInstances{};

		StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> directionalLightBuffers{};

		int maxDirectionalLights = 0;

    };
    
} // namespace CE::RPI

#include "DirectionalLightFeatureProcessor.rtti.h"