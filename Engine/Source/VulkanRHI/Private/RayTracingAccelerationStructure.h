#pragma once

namespace CE::Vulkan
{
 
	//! Vulkan RAII class for Ray Tracing Acceleration Structure (BLAS)
    class RayTracingAccelerationStructure
    {
        CE_NO_COPY(RayTracingAccelerationStructure)
    public:

        RayTracingAccelerationStructure(VulkanDevice* device);

        ~RayTracingAccelerationStructure();

    private:

        VulkanDevice* device = nullptr;

		VkAccelerationStructureKHR accelerationStructure = nullptr;
        
    };

} // namespace CE::Vulkan
