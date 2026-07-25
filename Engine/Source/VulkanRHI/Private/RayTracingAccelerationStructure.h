#pragma once

namespace CE::Vulkan
{
 
	//! Vulkan RAII class for Ray Tracing Acceleration Structure
    class RayTracingAccelerationStructure
    {
        CE_NO_COPY(RayTracingAccelerationStructure)
    public:

        RayTracingAccelerationStructure(Device* device, const VkAccelerationStructureCreateInfoKHR& createInfo);

        ~RayTracingAccelerationStructure();

        VkAccelerationStructureKHR GetHandle() const { return accelerationStructure; }

    private:

        Device* device = nullptr;

		VkAccelerationStructureKHR accelerationStructure = nullptr;
        
    };

} // namespace CE::Vulkan
