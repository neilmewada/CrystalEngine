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
		bool useDeviceAddressExt = false;

		switch (heapType)
		{
		case MemoryHeapType::Default:
			memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			useDeviceAddressExt = device->HasBufferDeviceAddressExt();
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

		VkMemoryAllocateFlagsInfo allocFlagsInfo{};
		allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		allocFlagsInfo.pNext = nullptr;

		if (useDeviceAddressExt)
		{
			allocInfo.pNext = &allocFlagsInfo;
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
			if (desc.compatibleMemoryTypes.value == 0 && ((memoryProps.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags))
			{
				allocatedMemoryTypeIndex = i;
				break;
			}
			else if ((desc.compatibleMemoryTypes.value & (1 << i)) && ((memoryProps.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags))
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

	MemoryHeap::MemoryHeap(Device* device, VkDeviceMemory allocation, VkMemoryPropertyFlags memoryPropertyFlags,
		u32 supportedMemoryTypeBitMask, u32 allocatedMemoryTypeIndex)
		: device(device)
		, allocation(allocation)
		, memoryPropertyFlags(memoryPropertyFlags)
		, supportedMemoryTypeBitMask(supportedMemoryTypeBitMask)
		, allocatedMemoryTypeIndex(allocatedMemoryTypeIndex)
	{

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
