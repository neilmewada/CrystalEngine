#pragma once

namespace CE::Vulkan
{
    
    class RayTracingBlas : public RHI::RayTracingBlas
    {
        CE_NO_COPY(RayTracingBlas)
	public:

        RayTracingBlas(Device* device, const RHI::RayTracingBlasDescriptor& desc);

		~RayTracingBlas() override;

		RayTracingAccelerationStructureOld* GetAccelerationStructure() const { return accelerationStructure; }

    private:

        Device* device = nullptr;

        Vulkan::Buffer* blasBuffer = nullptr;
        Vulkan::Buffer* scratchBuffer = nullptr;
        RayTracingAccelerationStructureOld* accelerationStructure = nullptr;

        Array<VkAccelerationStructureGeometryKHR> geometryDescriptors;
        Array<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos;
        VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};

        friend class CommandList;
	};

}