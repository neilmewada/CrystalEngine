
#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

	CommandQueue::CommandQueue(VulkanDevice* device, 
		u32 familyIndex, u32 queueIndex, 
		RHI::HardwareQueueClassMask queueMask, 
		VkQueue queue, bool presentSupported)
		: device(device)
		, familyIndex(familyIndex)
		, queueIndex(queueIndex)
		, queue(queue)
		, presentSupported(presentSupported)
	{
		this->queueMask = queueMask;
	}

	CommandQueue::~CommandQueue()
	{
		
	}

	bool CommandQueue::Execute(u32 count, RHI::CommandList** commandLists, RHI::Fence* fence)
	{
		if (count == 0)
			return true;

		SubmitInfo info{};
		info.commandBuffers.Resize(count);

		for (int i = 0; i < count; i++)
		{
			if (commandLists[i] == nullptr)
				return false;

			info.commandBuffers[i] = ((Vulkan::CommandList*)commandLists[i])->GetCommandBuffer();
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = info.commandBuffers.GetSize();
		submitInfo.pCommandBuffers = info.commandBuffers.GetData();

		LockGuard guard{ submissionMutex };

		if (fence != nullptr)
		{
			auto submitFence = ((Vulkan::Fence*)fence)->GetHandle();
			vkQueueSubmit(queue, 1, &submitInfo, submitFence);
		}
		else
		{
			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		}
		
		return true;
	}

	bool CommandQueue::Submit(u32 count, VkSubmitInfo* submitInfos, VkFence fence)
	{
		if (count == 0)
			return true;

		Array<SubmitInfo> infos{};
		List<VkSubmitInfo> vkSubmits{};
		infos.Resize(count);

		for (int i = 0; i < count; i++)
		{
			infos[i].commandBuffers.Resize(submitInfos[i].commandBufferCount);
			
			for (int j = 0; j < submitInfos[i].commandBufferCount; j++)
			{
				infos[i].commandBuffers[j] = submitInfos[i].pCommandBuffers[j];
			}

			infos[i].waitDstStageMask.Resize(submitInfos[i].waitSemaphoreCount);
			infos[i].waitSemaphores.Resize(submitInfos[i].waitSemaphoreCount);
			
			for (int j = 0; j < submitInfos[i].waitSemaphoreCount; j++)
			{
				infos[i].waitSemaphores[j] = submitInfos[i].pWaitSemaphores[j];
				infos[i].waitDstStageMask[j] = submitInfos[i].pWaitDstStageMask[j];
			}

			infos[i].signalSemaphores.Resize(submitInfos[i].signalSemaphoreCount);
			
			for (int j = 0; j < submitInfos[i].signalSemaphoreCount; j++)
			{
				infos[i].signalSemaphores[j] = submitInfos[i].pSignalSemaphores[j];
			}
		}

		for (int i = 0; i < count; i++)
		{
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = infos[i].commandBuffers.GetSize();
			submitInfo.pCommandBuffers = infos[i].commandBuffers.GetData();

			submitInfo.waitSemaphoreCount = infos[i].waitSemaphores.GetSize();
			submitInfo.pWaitSemaphores = infos[i].waitSemaphores.GetData();
			submitInfo.pWaitDstStageMask = infos[i].waitDstStageMask.GetData();
			submitInfo.signalSemaphoreCount = infos[i].signalSemaphores.GetSize();
			submitInfo.pSignalSemaphores = infos[i].signalSemaphores.GetData();

			vkSubmits.Add(submitInfo);
		}

		LockGuard guard{ submissionMutex };

		vkQueueSubmit(queue, count, vkSubmits.GetData(), fence);

		return true;
	}

} // namespace CE
