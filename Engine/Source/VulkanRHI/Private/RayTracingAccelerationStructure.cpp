#pragma once

#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

	RayTracingAccelerationStructure::RayTracingAccelerationStructure(VulkanDevice* device, const VkAccelerationStructureCreateInfoKHR& createInfo) 
		: device(device)
	{
		device->vkCreateAccelerationStructureKHR(device->GetHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &accelerationStructure);
		
	}

	RayTracingAccelerationStructure::~RayTracingAccelerationStructure()
	{
		if (accelerationStructure != nullptr)
		{
			device->vkDestroyAccelerationStructureKHR(device->GetHandle(), accelerationStructure, VULKAN_CPU_ALLOCATOR);
			accelerationStructure = nullptr;
		}

	}
} // namespace CE::Vulkan
