
#include "VulkanRHI.h"
#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    
    DescriptorPool::DescriptorPool()
    {
		
    }

    DescriptorPool::~DescriptorPool()
    {
		LockGuard guard{ vkPoolMutex };

        for (VkDescriptorPool pool : descriptorPools)
        {
            vkDestroyDescriptorPool(device->GetHandle(), pool, VULKAN_CPU_ALLOCATOR);
        }
        descriptorPools.Clear();
    }

	void DescriptorPool::Init(VulkanDevice* device, u32 initialPoolSize, u32 poolSizeIncrement)
	{
		this->device = device;

		VkDescriptorPoolSize poolSizes[] = {
			{.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = initialPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = initialPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = initialPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = initialPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = initialPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = initialPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = initialPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = initialPoolSize },
		};

		VkDescriptorPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.maxSets = initialPoolSize * 4;
		info.poolSizeCount = COUNTOF(poolSizes);
		info.pPoolSizes = poolSizes;
		info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;// | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

		VkDescriptorPool pool = nullptr;

		LockGuard guard{ vkPoolMutex };
		auto result = vkCreateDescriptorPool(device->GetHandle(), &info, VULKAN_CPU_ALLOCATOR, &pool);
		if (result != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to create vulkan descriptor pool. Error code: {}", (int)result);
			return;
		}

		descriptorPools.Add(pool);
	}

	List<VkDescriptorSet> DescriptorPool::Allocate(u32 numDescriptorSets, const List<VkDescriptorSetLayout>& setLayouts, VkDescriptorPool& outPool)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = numDescriptorSets;
		allocInfo.pSetLayouts = setLayouts.GetData();

		if (descriptorPools.IsEmpty())
		{
			Increment(Math::Max(incrementSize, numDescriptorSets));
		}

		allocInfo.descriptorPool = descriptorPools.GetLast();

		List<VkDescriptorSet> sets = List<VkDescriptorSet>(numDescriptorSets);

		VkResult result = VK_SUCCESS;

		{
			LockGuard guard{ vkPoolMutex };
			result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, sets.GetData());
		}

		if (result == VK_ERROR_OUT_OF_POOL_MEMORY ||
			result == VK_ERROR_FRAGMENTED_POOL)
		{
			// If we need a lot of descriptor sets allocated
			if (numDescriptorSets > initialSize)
			{
				Increment(Math::Max(incrementSize, numDescriptorSets));
				allocInfo.descriptorPool = descriptorPools.GetLast();

				LockGuard guard{ vkPoolMutex };
				result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, sets.GetData());
			}

			for (int i = descriptorPools.GetSize() - 1; i >= 0 && result != VK_SUCCESS; i--)
			{
				allocInfo.descriptorPool = descriptorPools[i];

				LockGuard guard{ vkPoolMutex };
				result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, sets.GetData());
			}

			if (result != VK_SUCCESS)
			{
				Increment(Math::Max(incrementSize, numDescriptorSets));
			}

			for (int i = descriptorPools.GetSize() - 1; i >= 0 && result != VK_SUCCESS; i--)
			{
				allocInfo.descriptorPool = descriptorPools[i];

				LockGuard guard{ vkPoolMutex };
				result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, sets.GetData());
			}
		}

		if (result != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to allocate descriptor sets! Error code {}", (int)result);
			return {};
		}
		
		outPool = allocInfo.descriptorPool;
		return sets;
	}

	List<VkDescriptorSet> DescriptorPool::Allocate(u32 numDescriptorSets, const List<VkDescriptorSetLayout>& setLayouts, VkDescriptorPool& outPool, 
		const List<uint32_t>& descriptorCounts)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = numDescriptorSets;
		allocInfo.pSetLayouts = setLayouts.GetData();

		// TODO: Implement dynamic array sizing!
		VkDescriptorSetVariableDescriptorCountAllocateInfo setCount{};
		setCount.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		setCount.descriptorSetCount = numDescriptorSets;
		setCount.pDescriptorCounts = descriptorCounts.GetData();

		allocInfo.pNext = &setCount;

		if (descriptorPools.IsEmpty())
		{
			Increment(Math::Max(incrementSize, numDescriptorSets));
		}
		allocInfo.descriptorPool = descriptorPools.GetLast();

		List<VkDescriptorSet> sets = List<VkDescriptorSet>(numDescriptorSets);

		VkResult result = VK_SUCCESS;

		{
			LockGuard guard{ vkPoolMutex };
			result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, sets.GetData());
		}

		if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
		{
			// If we need a lot of descriptor sets allocated
			if (numDescriptorSets > initialSize)
			{
				Increment(Math::Max(incrementSize, numDescriptorSets));
				allocInfo.descriptorPool = descriptorPools.GetLast();

				LockGuard guard{ vkPoolMutex };
				result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, sets.GetData());
			}

			for (int i = descriptorPools.GetSize() - 1; i >= 0 && result != VK_SUCCESS; i--)
			{
				allocInfo.descriptorPool = descriptorPools[i];

				LockGuard guard{ vkPoolMutex };
				result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, sets.GetData());
			}

			if (result != VK_SUCCESS)
			{
				Increment(Math::Max(incrementSize, numDescriptorSets));
			}

			for (int i = descriptorPools.GetSize() - 1; i >= 0 && result != VK_SUCCESS; i--)
			{
				allocInfo.descriptorPool = descriptorPools[i];

				LockGuard guard{ vkPoolMutex };
				result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, sets.GetData());
			}
		}

		if (result != VK_SUCCESS)
		{
			CE_LOG(Error, All, "Failed to allocate descriptor sets! Error code {}", (int)result);
			return {};
		}

		outPool = allocInfo.descriptorPool;
		return sets;
	}

	void DescriptorPool::Free(const List<VkDescriptorSet>& sets)
	{
		if (sets.IsEmpty())
			return;

		LockGuard guard{ vkPoolMutex };

		for (int i = descriptorPools.GetSize() - 1; i >= 0; i--)
		{
			auto result = vkFreeDescriptorSets(device->GetHandle(), descriptorPools[i], sets.GetSize(), sets.GetData());

			if (result == VK_SUCCESS)
				return;
		}
	}

    void DescriptorPool::Increment(u32 incrementSize)
    {
        VkDescriptorPoolSize poolSizes[] = {
            { .type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = incrementSize },
            { .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = incrementSize },
			{ .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = incrementSize },
            { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = incrementSize },
            { .type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = incrementSize },
            { .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = incrementSize },
			{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = incrementSize },
			{ .type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = incrementSize },
        };
        
        VkDescriptorPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.maxSets = incrementSize * 4;
        info.poolSizeCount = COUNTOF(poolSizes);
        info.pPoolSizes = poolSizes;
		info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;// | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        
        VkDescriptorPool pool = nullptr;

		LockGuard guard{ vkPoolMutex };
        auto result = vkCreateDescriptorPool(device->GetHandle(), &info, VULKAN_CPU_ALLOCATOR, &pool);
        if (result != VK_SUCCESS)
        {
            CE_LOG(Error, All, "Failed to create vulkan descriptor pool. Error code: {}", (int)result);
            return;
        }
        
        descriptorPools.Add(pool);
    }

} // namespace CE

