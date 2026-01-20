#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    // FIXME: Rewrite the whole fence class
    
    Fence::Fence(VulkanDevice* device, uint64_t initialValue) : RHI::Fence(initialValue)
    {
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = 0;
        if (initialValue != 0)
            createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vkCreateFence(device->GetHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &fence);
    }

    Fence::~Fence()
    {
        vkDestroyFence(device->GetHandle(), fence, VULKAN_CPU_ALLOCATOR);
    }

} // namespace CE::Vulkan
