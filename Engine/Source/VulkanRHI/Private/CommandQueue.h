#pragma once

#include "VulkanRHI.h"
#include "vulkan/vulkan.h"

namespace CE::Vulkan
{
    class Device;

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

        CommandQueue(VulkanDevice* device, u32 familyIndex, u32 queueIndex, RHI::HardwareQueueClassMask queueMask, RHI::HardwareQueueClass queueClass, VkQueue queue, bool presentSupported);
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

        bool Submit(u32 count, VkSubmitInfo* submitInfos, VkFence fence);

        bool Submit(const CommandQueueSubmission& submission) override;
        
		SharedMutex& GetMutex() { return submissionMutex; }

    private:

        struct SubmitBatch
        {
            List<SubmitInfo> submitInfos{};
            VkFence fence = nullptr;
        };

        SharedMutex submissionMutex{};

        Device* device;
        u32 familyIndex;
        u32 queueIndex;
        VkQueue queue;
		bool presentSupported = false;
		VkCommandPool commandPool = nullptr;

        friend class Device;
        friend class FrameGraphExecuter;
    };
    
} // namespace CE
