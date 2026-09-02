#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	AliasedHeap::AliasedHeap(Device* device, const AliasedHeapDescriptor& desc) 
		: RHI::AliasedHeap(desc)
		, device(device)
	{
		debugName = desc.debugName;
		heapSize = desc.allocationSize;
		usageFlags = desc.usageFlags;
		supportedMemoryTypeBitMask = 0;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = heapSize;

		memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkMemoryAllocateFlagsInfo allocFlagsInfo{};
		allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		allocFlagsInfo.pNext = nullptr;

		if (device->HasBufferDeviceAddressExt())
		{
			allocInfo.pNext = &allocFlagsInfo;
		}

		VkPhysicalDeviceMemoryProperties memoryProps = device->GetMemoryProperties();
		const u32 compatibleMemoryTypes = desc.compatibleMemoryTypes.value;
		bool memoryTypeFound = false;

		for (int i = 0; i < memoryProps.memoryTypeCount; i++)
		{
			const u32 memoryTypeBit = 1u << i;
			if ((compatibleMemoryTypes & memoryTypeBit) != 0 &&
				(memoryProps.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
			{
				supportedMemoryTypeBitMask |= memoryTypeBit;
				if (!memoryTypeFound)
				{
					allocatedMemoryTypeIndex = i;
					memoryTypeFound = true;
				}
			}
		}

		if (!memoryTypeFound)
		{
			CE_LOG(Error, All, "No compatible device-local memory type found for aliased heap {}", debugName);
			return;
		}

		allocInfo.memoryTypeIndex = allocatedMemoryTypeIndex;

		VkDeviceMemory memoryAllocation{};

		auto result = vkAllocateMemory(device->GetHandle(), &allocInfo, VULKAN_CPU_ALLOCATOR, &memoryAllocation);
		if (result != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to allocate heap memory of size {}: {}", heapSize, debugName);
			return;
		}

		allocation = new Vulkan::MemoryHeap(device, memoryAllocation, memoryPropertyFlags, supportedMemoryTypeBitMask, allocatedMemoryTypeIndex);
	}

	AliasedHeap::~AliasedHeap()
	{
		delete allocation; allocation = nullptr;
	}
} // namespace CE::Vulkan
