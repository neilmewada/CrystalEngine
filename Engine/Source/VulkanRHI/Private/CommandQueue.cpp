
#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

	CommandQueue::CommandQueue(Device* device, 
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

		submissionMutex.Lock();
		if (fence != nullptr)
		{
			auto submitFence = ((Vulkan::Fence*)fence)->GetHandle();
			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		}
		else
		{
			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		}
		submissionMutex.Unlock();
		
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

		submissionMutex.Lock();
		vkQueueSubmit(queue, count, vkSubmits.GetData(), fence);
		submissionMutex.Unlock();

		return true;
	}

	bool CommandQueue::Submit(const CommandQueueSubmission& submission)
	{
		if (submission.numCommandLists == 0 || submission.commandLists == nullptr)
		{
			return false;
		}

		List<VkCommandBuffer> commandBuffers{};
		commandBuffers.Reserve(submission.numCommandLists);

		for (u32 i = 0; i < submission.numCommandLists; i++)
		{
			if (submission.commandLists[i] == nullptr)
				return false;
			commandBuffers.Add(((Vulkan::CommandList*)submission.commandLists[i])->GetCommandBuffer());
		}

		List<VkSemaphore> waitSemaphores{};
		List<VkPipelineStageFlags> waitStages{};
		List<uint64_t> waitValues{};

		List<VkSemaphore> signalSemaphores{};
		List<uint64_t> signalValues{};

		List<VkSemaphore> presentWaitSemaphores{};

		waitSemaphores.Reserve(submission.numPresentSwapChains + 1);
		waitStages.Reserve(submission.numPresentSwapChains + 1);
		waitValues.Reserve(submission.numPresentSwapChains + 1);

		signalSemaphores.Reserve(submission.numPresentSwapChains + 1);
		signalValues.Reserve(submission.numPresentSwapChains + 1);

		presentWaitSemaphores.Reserve(submission.numPresentSwapChains);

		List<VkSwapchainKHR> swapChains{};
		List<uint32_t> imageIndices{};
		
		swapChains.Reserve(submission.numPresentSwapChains);
		imageIndices.Reserve(submission.numPresentSwapChains);

		for (u32 i = 0; i < submission.numPresentSwapChains; ++i)
		{
			if (submission.presentSwapChains[i] == nullptr)
				return false;

			Vulkan::SwapChain* presentSwapChain = (Vulkan::SwapChain*)submission.presentSwapChains[i];
			swapChains.Add(presentSwapChain->GetHandle());
			imageIndices.Add(presentSwapChain->currentImageIndex);

			VkSemaphore imageAcquiredSemaphore = presentSwapChain->imageAcquiredSemaphores[presentSwapChain->currentImageAcquiredSemaphoreIndex];
			waitSemaphores.Add(imageAcquiredSemaphore);
			waitStages.Add(presentSwapChain->IsFrameBufferOnly() ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
			waitValues.Add(0);

			VkSemaphore renderFinishedSemaphore = presentSwapChain->renderFinishedSemaphores[presentSwapChain->currentImageIndex];
			signalSemaphores.Add(renderFinishedSemaphore);
			signalValues.Add(0);

			presentWaitSemaphores.Add(renderFinishedSemaphore);
		}
		
		if (submission.waitFence)
		{
			Vulkan::Fence* waitFence = (Vulkan::Fence*)submission.waitFence;
			waitSemaphores.Add(waitFence->GetHandle());
			waitValues.Add(submission.waitFenceValue);

			VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			switch (submission.waitFenceStage)
			{
			case ResourceState::Undefined:
			case ResourceState::General:
				waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				break;
			case ResourceState::CopyDestination:
			case ResourceState::CopySource:
				waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				break;
			case ResourceState::DepthWrite:
			case ResourceState::DepthRead:
				waitStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				break;
			case ResourceState::FragmentShaderResource:
				waitStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				break;
			case ResourceState::NonFragmentShaderResource:
				waitStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
					VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				break;
			case ResourceState::ColorOutput:
				waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				break;
			case ResourceState::VertexBuffer:
			case ResourceState::IndexBuffer:
				waitStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
				break;
			case ResourceState::ConstantBuffer:
				waitStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				break;
			case ResourceState::Present:
				waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				break;
			case ResourceState::ShaderWrite:
				waitStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				break;
			case ResourceState::BlitSource:
			case ResourceState::BlitDestination:
				waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				break;
			}

			waitStages.Add(waitStage);
		}

		if (submission.signalFence)
		{
			Vulkan::Fence* signalFence = (Vulkan::Fence*)submission.signalFence;
			signalSemaphores.Add(signalFence->GetHandle());
			signalValues.Add(submission.signalFenceValue);
		}

		VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmitInfo{};
		timelineSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timelineSemaphoreSubmitInfo.waitSemaphoreValueCount = static_cast<uint32_t>(waitValues.GetSize());
		timelineSemaphoreSubmitInfo.pWaitSemaphoreValues = waitValues.IsEmpty() ? nullptr : waitValues.GetData();
		timelineSemaphoreSubmitInfo.signalSemaphoreValueCount = static_cast<uint32_t>(signalValues.GetSize());
		timelineSemaphoreSubmitInfo.pSignalSemaphoreValues = signalValues.IsEmpty() ? nullptr : signalValues.GetData();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = &timelineSemaphoreSubmitInfo;

		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.GetSize());
		submitInfo.pWaitSemaphores = waitSemaphores.IsEmpty() ? nullptr : waitSemaphores.GetData();
		submitInfo.pWaitDstStageMask = waitStages.IsEmpty() ? nullptr : waitStages.GetData();

		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.GetSize());
		submitInfo.pSignalSemaphores = signalSemaphores.IsEmpty() ? nullptr : signalSemaphores.GetData();

		submitInfo.commandBufferCount = commandBuffers.GetSize();
		submitInfo.pCommandBuffers = commandBuffers.GetData();

		VkResult result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
		{
			return false;
		}

		if (submission.numPresentSwapChains > 0)
		{
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.pNext = nullptr;

			presentInfo.swapchainCount = submission.numPresentSwapChains;

			presentInfo.pImageIndices = imageIndices.GetData();
			presentInfo.pSwapchains = swapChains.GetData();

			presentInfo.waitSemaphoreCount = presentWaitSemaphores.GetSize();
			presentInfo.pWaitSemaphores = presentWaitSemaphores.GetData();

			result = vkQueuePresentKHR(queue, &presentInfo);

			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				for (u32 i = 0; i < submission.numPresentSwapChains; i++)
				{
					Vulkan::SwapChain* presentSwapChain = (Vulkan::SwapChain*)submission.presentSwapChains[i];

					presentSwapChain->shouldRebuild = true;
				}

				return false;
			}

			if (result == VK_SUBOPTIMAL_KHR)
			{
				for (u32 i = 0; i < submission.numPresentSwapChains; i++)
				{
					Vulkan::SwapChain* presentSwapChain = (Vulkan::SwapChain*)submission.presentSwapChains[i];

					presentSwapChain->shouldRebuild = true;
				}
			}

			if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			{
				return false;
			}
		}
		
		return true;
	}

} // namespace CE
