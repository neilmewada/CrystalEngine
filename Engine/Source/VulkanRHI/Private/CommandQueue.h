#pragma once

#include "VulkanRHI.h"
#include "vulkan/vulkan.h"

namespace CE::Vulkan
{
    class VulkanDevice;

    class CommandQueue : public RHI::CommandQueue
    {
    public:

        struct SubmitInfo
        {
            List<VkCommandBuffer> commandBuffers{};
            List<VkSemaphore> signalSemaphores{};
            List<VkSemaphore> waitSemaphores{};
            List<VkPipelineStageFlags> waitDstStageMask{};
        };

        CommandQueue(VulkanDevice* device, u32 familyIndex, u32 queueIndex, RHI::HardwareQueueClassMask queueMask, VkQueue queue, bool presentSupported);
        virtual ~CommandQueue();

		inline VkQueue GetHandle() const
        {
            return queue;
        }

		inline u32 GetFamilyIndex() const
        {
            return familyIndex;
        }

        inline u32 GetQueueIndex() const
        {
            return queueIndex;
        }
		
		inline bool IsPresentSupported() const
		{
			return presentSupported;
		}

		inline VkCommandPool GetCommandPool() const
		{
			return commandPool;
		}

        virtual bool Execute(u32 count, RHI::CommandList** commandLists, RHI::Fence* fence = nullptr) override;

        bool Submit(u32 count, VkSubmitInfo* submitInfos, VkFence fence);

		SharedMutex& GetMutex() { return submissionMutex; }

    private:

        struct SubmitBatch
        {
            List<SubmitInfo> submitInfos{};
            VkFence fence = nullptr;
        };

        SharedMutex submissionMutex{};

        VulkanDevice* device;
        u32 familyIndex;
        u32 queueIndex;
        VkQueue queue;
		bool presentSupported = false;
		VkCommandPool commandPool = nullptr;

        friend class VulkanDevice;
        friend class FrameGraphExecuter;
    };
    
} // namespace CE
