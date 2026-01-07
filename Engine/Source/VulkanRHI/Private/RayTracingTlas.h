#pragma once

namespace CE::Vulkan
{

    class RayTracingTlas : public RHI::RayTracingTlas
    {
        CE_NO_COPY(RayTracingTlas)
    public:

        RayTracingTlas(VulkanDevice* device, const RHI::RayTracingTlasDescriptor& desc);

		~RayTracingTlas() override;

		RayTracingAccelerationStructure* GetAccelerationStructure() const { return accelerationStructure; }

    private:

        VulkanDevice* device = nullptr;

		Vulkan::Buffer* tlasBuffer = nullptr;
        Vulkan::Buffer* scratchBuffer = nullptr;
        RayTracingAccelerationStructure* accelerationStructure = nullptr;
        Array<VkAccelerationStructureInstanceKHR> vkInstances;
        Vulkan::Buffer* tlasInstancesBuffer = nullptr;

        VkAccelerationStructureGeometryKHR geometry{};
        VkAccelerationStructureBuildRangeInfoKHR offsetInfo{};
		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
        u32 instanceCount = 0;

		friend class CommandList;
    };
    
} // namespace CE::Vulkan
