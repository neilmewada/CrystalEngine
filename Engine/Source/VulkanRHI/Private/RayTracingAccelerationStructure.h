#pragma once

namespace CE::Vulkan
{

    class RayTracingAccelerationStructure : public RHI::RayTracingAccelerationStructure
    {
    public:

        RayTracingAccelerationStructure(Device* device, const RHI::RayTracingBlasDescriptor& blasDescriptor);
        RayTracingAccelerationStructure(Device* device, const RHI::RayTracingTlasDescriptor& tlasDescriptor);

        ~RayTracingAccelerationStructure();

    private:

        Device* device = nullptr;
        VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
        VkBuildAccelerationStructureFlagsKHR accelerationStructureFlags = 0;

    };
 
	//! Vulkan RAII class for Ray Tracing Acceleration Structure
    class RayTracingAccelerationStructureOld
    {
        CE_NO_COPY(RayTracingAccelerationStructureOld)
    public:

        RayTracingAccelerationStructureOld(Device* device, const VkAccelerationStructureCreateInfoKHR& createInfo);

        ~RayTracingAccelerationStructureOld();

        VkAccelerationStructureKHR GetHandle() const { return accelerationStructure; }

    private:

        Device* device = nullptr;

		VkAccelerationStructureKHR accelerationStructure = nullptr;
        
    };

} // namespace CE::Vulkan
