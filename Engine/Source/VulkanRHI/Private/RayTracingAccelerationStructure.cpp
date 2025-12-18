#pragma once

#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	RayTracingAccelerationStructure::RayTracingAccelerationStructure(VulkanDevice* device) : device(device)
	{
		device->vkCreateAccelerationStructureKHR(device->GetHandle(), nullptr, VULKAN_CPU_ALLOCATOR, &accelerationStructure);
		
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
