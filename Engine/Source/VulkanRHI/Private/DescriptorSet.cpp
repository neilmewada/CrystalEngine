#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    DescriptorSet::DescriptorSet(Device* device, VkDescriptorSetLayout setLayout, const RHI::ShaderResourceGroupLayout& srgLayout, const String& name)
		: device(device)
    {
		this->srgLayout = srgLayout;
		this->srgManager = device->GetShaderResourceManager();
		pool = device->GetDescriptorPool();
		setNumber = srgManager->GetDescriptorSetNumber(srgLayout.srgType);

		auto allocatedSets = pool->Allocate(1, { setLayout }, descriptorPool);
		if (allocatedSets.IsEmpty())
		{
			failed = true;
			CE_LOG(Error, All, "Failed to allocate descriptor set");
			return;
		}

		descriptorSet = allocatedSets[0];

		device->SetObjectDebugName((uint64_t)descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, name.GetCString());
    }

    DescriptorSet::DescriptorSet(Device* device, VkDescriptorSetLayout setLayout,
	    const RHI::ShaderResourceGroupLayout& srgLayout, u32 arrayCount, const String& name)
		: device(device)
    {
		this->srgLayout = srgLayout;
		this->srgManager = device->GetShaderResourceManager();
		pool = device->GetDescriptorPool();
		setNumber = srgManager->GetDescriptorSetNumber(srgLayout.srgType);

		auto allocatedSets = pool->Allocate(1, { setLayout }, descriptorPool, { arrayCount });
		if (allocatedSets.IsEmpty())
		{
			failed = true;
			CE_LOG(Error, All, "Failed to allocate descriptor set");
			return;
		}

		descriptorSet = allocatedSets[0];

		device->SetObjectDebugName((uint64_t)descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, name.GetCString());
    }

    DescriptorSet::~DescriptorSet()
    {
		pool->Free({ descriptorSet });
		//vkFreeDescriptorSets(device->GetHandle(), descriptorPool, 1, &descriptorSet);
		descriptorSet = nullptr;
    }

} // namespace CE::Vulkan
