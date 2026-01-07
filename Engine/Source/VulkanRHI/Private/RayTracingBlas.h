#pragma once

namespace CE::Vulkan
{
    
    class RayTracingBlas : public RHI::RayTracingBlas
    {
        CE_NO_COPY(RayTracingBlas)
	public:

        RayTracingBlas(VulkanDevice* device, const RHI::RayTracingBlasDescriptor& desc);

		~RayTracingBlas() override;

		RayTracingAccelerationStructure* GetAccelerationStructure() const { return accelerationStructure; }

    private:

        VulkanDevice* device = nullptr;

        Vulkan::Buffer* blasBuffer = nullptr;
        Vulkan::Buffer* scratchBuffer = nullptr;
        RayTracingAccelerationStructure* accelerationStructure = nullptr;

        Array<VkAccelerationStructureGeometryKHR> geometryDescriptors;
        Array<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos;
        VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};

        friend class CommandList;
	};

}