#pragma once

#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	static VkFormat ToVkFormat(RHI::VertexAttributeDataType dataType)
	{
		switch (dataType)
		{
		case VertexAttributeDataType::Float:
			return VK_FORMAT_R32_SFLOAT;
			break;
		case VertexAttributeDataType::Float2:
			return VK_FORMAT_R32G32_SFLOAT;
			break;
		case VertexAttributeDataType::Float3:
			return VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case VertexAttributeDataType::Float4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case VertexAttributeDataType::Int:
			return VK_FORMAT_R32_SINT;
			break;
		case VertexAttributeDataType::Int2:
			return VK_FORMAT_R32G32_SINT;
			break;
		case VertexAttributeDataType::Int3:
			return VK_FORMAT_R32G32B32_SINT;
			break;
		case VertexAttributeDataType::Int4:
			return VK_FORMAT_R32G32B32A32_SINT;
			break;
		case VertexAttributeDataType::UInt:
			return VK_FORMAT_R32_UINT;
			break;
		case VertexAttributeDataType::UInt2:
			return VK_FORMAT_R32G32_UINT;
			break;
		case VertexAttributeDataType::UInt3:
			return VK_FORMAT_R32G32B32_UINT;
			break;
		case VertexAttributeDataType::UInt4:
			return VK_FORMAT_R32G32B32A32_UINT;
			break;
		case VertexAttributeDataType::Undefined:
			break;
		case VertexAttributeDataType::Char4:
			return VK_FORMAT_R8G8B8A8_SNORM;
			break;
		case VertexAttributeDataType::UChar4:
			return VK_FORMAT_R8G8B8A8_UNORM;
			break;
		}

		return VK_FORMAT_UNDEFINED;
	}

	static VkBuildAccelerationStructureFlagsKHR GetVkAccelerationStructureBuildFlags(RHI::RayTracingAccelerationStructureFlags buildFlags)
	{
		VkBuildAccelerationStructureFlagsKHR flags = 0;

		if (EnumHasFlag(buildFlags, RayTracingAccelerationStructureFlags::AllowCompaction))
		{
			flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
		}

		if (EnumHasFlag(buildFlags, RayTracingAccelerationStructureFlags::AllowUpdate))
		{
			flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		}

		if (EnumHasFlag(buildFlags, RayTracingAccelerationStructureFlags::FastBuild))
		{
			flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
		}

		if (EnumHasFlag(buildFlags, RayTracingAccelerationStructureFlags::FastTrace))
		{
			flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		}

		return flags;
	}

	RayTracingAccelerationStructure::RayTracingAccelerationStructure(Device* device, const RHI::RayTracingBlasDescriptor& blasDescriptor)
		: device(device)
	{
		accelerationStructureType = RayTracingAccelerationStructureType::BottomLevel;
		flags = blasDescriptor.flags;
		accelerationStructureFlags = GetVkAccelerationStructureBuildFlags(flags);

	}

	RayTracingAccelerationStructure::RayTracingAccelerationStructure(Device* device, const RHI::RayTracingTlasDescriptor& tlasDescriptor)
		: device(device)
	{
		accelerationStructureType = RayTracingAccelerationStructureType::TopLevel;
		flags = tlasDescriptor.flags;
		accelerationStructureFlags = GetVkAccelerationStructureBuildFlags(flags);


	}

	RayTracingAccelerationStructure::~RayTracingAccelerationStructure()
	{
		
	}

	RayTracingAccelerationStructureOld::RayTracingAccelerationStructureOld(Device* device, const VkAccelerationStructureCreateInfoKHR& createInfo) 
		: device(device)
	{
		device->vkCreateAccelerationStructureKHR(device->GetHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &accelerationStructure);
		
	}

	RayTracingAccelerationStructureOld::~RayTracingAccelerationStructureOld()
	{
		if (accelerationStructure != nullptr)
		{
			device->vkDestroyAccelerationStructureKHR(device->GetHandle(), accelerationStructure, VULKAN_CPU_ALLOCATOR);
			accelerationStructure = nullptr;
		}

	}
} // namespace CE::Vulkan
