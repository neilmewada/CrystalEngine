#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	RayTracingTlas::RayTracingTlas(VulkanDevice* device, const RHI::RayTracingTlasDescriptor& desc)
		: RHI::RayTracingTlas(desc)
		, device(device)
	{
		instanceCount = desc.instances.GetSize();

		{
			Array<VkAccelerationStructureInstanceKHR> instanceData{};
			instanceData.Reserve(instanceCount);

			for (const auto& instance : desc.instances)
			{
				VkAccelerationStructureInstanceKHR& instanceInfo = instanceData.EmplaceBack();
				instanceInfo.mask = instance.instanceMask;
			}
		}
	}

	RayTracingTlas::~RayTracingTlas()
	{
		
	}
} // namespace CE::Vulkan
