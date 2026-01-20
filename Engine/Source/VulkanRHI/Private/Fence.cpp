#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    // FIXME: Rewrite the whole fence class
    
    Fence::Fence(Device* device, uint64_t initialValue) : RHI::Fence(initialValue), device(device)
    {
        VkSemaphoreTypeCreateInfo timelineCreateInfo{};
        timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineCreateInfo.pNext = nullptr;
        timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineCreateInfo.initialValue = initialValue;

        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = &timelineCreateInfo;
    	createInfo.flags = 0;

        vkCreateSemaphore(device->GetHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &semaphore);
    }

    Fence::~Fence()
    {
        vkDestroySemaphore(device->GetHandle(), semaphore, VULKAN_CPU_ALLOCATOR);
    }

    void Fence::RefreshCompletedValue()
    {
        uint64_t value = 0;
        if (vkGetSemaphoreCounterValue(device->GetHandle(), semaphore, &value) == VK_SUCCESS)
        {
            SetCompletedValue(value);
		}

    }

    bool Fence::WaitCPU(uint64_t value, uint64_t timeoutNs)
    {
        if (GetCompletedValue() >= value)
            return true;

        VkSemaphoreWaitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &semaphore;
        waitInfo.pValues = &value;

        VkResult result = vkWaitSemaphores(device->GetHandle(), &waitInfo, timeoutNs);
        RefreshCompletedValue();
		return result == VK_SUCCESS;
    }

} // namespace CE::Vulkan
