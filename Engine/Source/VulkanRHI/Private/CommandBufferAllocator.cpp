#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

	Array<CommandBufferAllocator::Context> CommandBufferAllocator::commandAllocatorContexts{};

    CommandBufferAllocator::CommandBufferAllocator(Device* device)
		: device(device)
		, threadedAllocator()
    {
		commandAllocatorContexts.Add({ this, device });

		
    }

    CommandBufferAllocator::~CommandBufferAllocator()
    {
		threadedAllocator.Clear();

		commandAllocatorContexts.Pop();
    }

	VkCommandPool CommandBufferAllocator::Allocate(u32 count, VkCommandBuffer* outBuffers, VkCommandBufferLevel level, u32 queueFamilyIndex)
	{
		if (count == 0)
			return nullptr;

		Allocator& alloc = threadedAllocator.GetStorage();
		VkCommandPool commandPool = alloc.GetOrCreate(queueFamilyIndex);
		if (commandPool == nullptr)
			return nullptr;
		
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = count;
		allocInfo.level = level;
		
		auto result = vkAllocateCommandBuffers(device->GetHandle(), &allocInfo, outBuffers);
		if (result != VK_SUCCESS)
			return nullptr;
		
		return commandPool;
	}

	void CommandBufferAllocator::Free(VkCommandPool pool, u32 count, VkCommandBuffer* buffers)
	{
		vkFreeCommandBuffers(device->GetHandle(), pool, count, buffers);
	}

	CommandBufferAllocator::Allocator::Allocator()
	{
		if (commandAllocatorContexts.IsEmpty())
		{
			valid = false;
			return;
		}

		context = commandAllocatorContexts.Top();
		valid = true;
	}

	CommandBufferAllocator::Allocator::~Allocator()
	{
		for (auto pool : commandPools)
		{
			vkDestroyCommandPool(context.device->GetHandle(), pool.handle, VULKAN_CPU_ALLOCATOR);
		}
		commandPools.Clear();

		valid = false;
		context = {};
	}

	VkCommandPool CommandBufferAllocator::Allocator::GetOrCreate(u32 queueFamilyIndex)
	{
		// Find if pool already exists for the given family index
		for (auto pool : commandPools)
		{
			if (pool.queueFamilyIndex == queueFamilyIndex)
				return pool.handle;
		}

		// Otherwise, create and store one for the given family index

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = queueFamilyIndex;

		VkCommandPool handle = nullptr;
		auto result = vkCreateCommandPool(context.device->GetHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &handle);
		if (result == VK_SUCCESS)
		{
			commandPools.Add({ handle, queueFamilyIndex });
		}

		return handle;
	}

} // namespace CE::Vulkan
