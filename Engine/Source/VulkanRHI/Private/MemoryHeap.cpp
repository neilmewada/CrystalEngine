#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	MemoryHeap::MemoryHeap(Device* device, const RHI::MemoryHeapDescriptor& desc)
		: device(device)
	{
		debugName = desc.debugName;
		heapType = desc.heapType;
		heapSize = desc.allocationSize;
		usageFlags = desc.usageFlags;
		supportedMemoryTypeBitMask = 0;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = heapSize;

		bool useUnifiedMemory = (device->IsUnifiedMemoryArchitecture() || device->SupportsReBar()) && (usageFlags == RHI::MemoryHeapUsageFlags::Buffer);

		switch (heapType)
		{
		case MemoryHeapType::Default:
			memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		case MemoryHeapType::Upload:
			memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			break;
		case MemoryHeapType::ReadBack:
			memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			break;
		}

		if (useUnifiedMemory)
		{
			memoryPropertyFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}
		
		VkPhysicalDeviceMemoryProperties memoryProps = device->GetMemoryProperties();

		for (int i = 0; i < memoryProps.memoryTypeCount; i++)
		{
			if ((memoryProps.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
			{
				supportedMemoryTypeBitMask |= (1 << i);
			}
		}

		for (int i = 0; i < memoryProps.memoryTypeCount; i++)
		{
			if (desc.flags == 0 && ((memoryProps.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags))
			{
				allocatedMemoryTypeIndex = i;
				break;
			}
			else if ((desc.flags & (1 << i)) && ((memoryProps.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags))
			{
				allocatedMemoryTypeIndex = i;
				break;
			}
		}

		allocInfo.memoryTypeIndex = allocatedMemoryTypeIndex;
		
		auto result = vkAllocateMemory(device->GetHandle(), &allocInfo, VULKAN_CPU_ALLOCATOR, &allocation);
		if (result != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to allocate memory of size {}: {}", heapSize, debugName);
			return;
		}
	}

	MemoryHeap::~MemoryHeap()
	{
		if (allocation != nullptr)
        {
            vkFreeMemory(device->GetHandle(), allocation, VULKAN_CPU_ALLOCATOR);
            allocation = nullptr;
        }
	}

	bool MemoryHeap::SupportsOptimalImageTiling()
	{
		// Optimal image tiling is supported only on Device local memory
		if (memoryPropertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			return true;
		return false;
	}

	bool MemoryHeap::BindBuffer(Buffer* buffer, VkDeviceSize offset)
	{
		auto result = vkBindBufferMemory(device->GetHandle(), buffer->GetBuffer(), allocation, offset);
		if (result != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to allocate buffer in MemoryHeap {}", debugName);
			return false;
		}

		allocatedRanges.Add({ offset, buffer->GetBufferSize() });

		return true;
	}

} // namespace CE::Vulkan
