#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

    class QueueAllocator
    {
    public:
        
        void Init()
        {
            for (int familyIdx = 0; familyIdx < queuesByFamily.GetSize(); familyIdx++)
            {
				for (CommandQueue* queue : queuesByFamily[familyIdx])
				{
					auto mask = queue->GetQueueMask();
					if (EnumHasFlag(mask, RHI::HardwareQueueClassMask::Graphics))
					{
						graphicsQueues.Add(queue);
					}
					else if (EnumHasFlag(mask, RHI::HardwareQueueClassMask::Compute))
					{
						computeQueues.Add(queue);
					}
					else if (EnumHasFlag(mask, RHI::HardwareQueueClassMask::Transfer))
					{
						transferQueues.Add(queue);
					}
				}
            }
        }
        
        CommandQueue* Acquire(int trackNumber, RHI::HardwareQueueClass queueClass, bool present = false)
        {
			if (trackNumber < graphicsQueues.GetSize())
				return graphicsQueues[trackNumber];
			else if (queueClass == RHI::HardwareQueueClass::Compute && (trackNumber - graphicsQueues.GetSize()) < computeQueues.GetSize())
				return computeQueues[trackNumber - graphicsQueues.GetSize()];
            return graphicsQueues.GetLast();
        }
        
		Array<CommandQueue*> graphicsQueues{};
		Array<CommandQueue*> computeQueues{};
		Array<CommandQueue*> transferQueues{};
        
        CommandQueue* primaryQueue = nullptr;
        CommandQueue* presentQueue = nullptr;
        Array<CommandQueue*> queues{};
        HashMap<int, Array<CommandQueue*>> queuesByFamily{};
    };
    
	FrameGraphCompiler::FrameGraphCompiler(VulkanDevice* device) : device(device)
	{

	}

	FrameGraphCompiler::~FrameGraphCompiler()
	{
		vkDeviceWaitIdle(device->GetHandle());

		DestroySyncObjects();
	}

	void FrameGraphCompiler::CompileScopesInternal(const RHI::FrameGraphCompileRequest& compileRequest)
	{
		ZoneScoped;

		//vkDeviceWaitIdle(device->GetHandle());

		// Queue allocation logic...

		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;
        QueueAllocator queueAllocator{};
        queueAllocator.queuesByFamily = device->queuesByFamily;
        queueAllocator.presentQueue = device->presentQueue;
        queueAllocator.primaryQueue = device->primaryGraphicsQueue;
        queueAllocator.queues = device->queues;
        queueAllocator.Init();
		
        std::function<void(RHI::Scope*, int)> visitor = [&](RHI::Scope* rhiScope, int i)
        {
            Vulkan::Scope* scope = (Vulkan::Scope*)rhiScope;
            if (scope->queue == nullptr)
			{
#if PLATFORM_MAC
                i = 0; // Temp code to force same queue
#endif
				scope->queue = queueAllocator.Acquire(i, scope->queueClass, scope->PresentsSwapChain());
			}

			bool swapChainFound = false;

			for (auto attachment : scope->attachments)
			{
				RHI::FrameAttachment* frameAttachment = attachment->GetFrameAttachment();
				if (frameAttachment->IsSwapChainAttachment())
				{
					swapChainFound = true;
					RHI::SwapChain* swapChain = ((RHI::SwapChainFrameAttachment*)frameAttachment)->GetSwapChain();
					if (!scope->swapChainsUsedByAttachments.Exists(swapChain))
						scope->swapChainsUsedByAttachments.Add(swapChain);
				}
			}

			if (!swapChainFound)
			{
				scope->swapChainsUsedByAttachments.Clear();
			}
            
            for (auto consumer : frameGraph->nodes[scope->id].consumers)
            {
				visitor(consumer, i);
				i++;
            }
        };
        
        int trackNumber = 0;
        for (RHI::Scope* scope : frameGraph->producers)
        {
			visitor(scope, trackNumber);
			trackNumber++;
        }

		trackNumber = 0;
	}

	void FrameGraphCompiler::CompileInternal(const RHI::FrameGraphCompileRequest& compileRequest)
	{
		ZoneScoped;

		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;

		vkDeviceWaitIdle(device->GetHandle());

		numFramesInFlight = compileRequest.numFramesInFlight;
		imageCount = numFramesInFlight;

		// If frame-graph presents at least 1 swap-chain
		bool presentSwapChains = false;

		if (frameGraph->presentSwapChains.NotEmpty())
		{
			imageCount = frameGraph->presentSwapChains[0]->GetImageCount();
			numFramesInFlight = imageCount;
			presentSwapChains = true;
		}

		for (auto scope : frameGraph->scopes)
		{
			delete scope->passShaderResourceGroup;
			scope->passShaderResourceGroup = nullptr;
			delete scope->subpassShaderResourceGroup;
			scope->subpassShaderResourceGroup = nullptr;
		}
		
		// Compile sync objects for individual scopes
		for (auto scope : frameGraph->scopes)
		{
			scope->Compile(compileRequest);
		}

		for (auto& fences : graphExecutionFences)
		{
			fences.Clear();
		}

		for (auto rhiScope : frameGraph->endScopes)
		{
			Vulkan::Scope* scope = (Vulkan::Scope*)rhiScope;
			for (int i = 0; i < imageCount; i++)
			{
                Vulkan::Scope* firstScope = scope;
                while (firstScope->prev != nullptr)
                {
                    firstScope = (Vulkan::Scope*)firstScope->prev;
                }
				graphExecutionFences[i].Add(firstScope->renderFinishedFences[i]);
			}
		}

		// Compile cross queue dependencies, i.e. Wait Semaphores
		CompileCrossQueueDependencies(compileRequest);

		// ---------------------------
		// Destroy old objects

		DestroySyncObjects();

		// ----------------------------
		// Create new objects

		if (presentSwapChains)
		{
			for (int i = 0; i < imageCount; i++)
			{
				for (int j = 0; j < frameGraph->presentSwapChains.GetSize(); j++)
				{
					VkSemaphoreCreateInfo semaphoreCI{};
					semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

					VkSemaphore semaphore = nullptr;
					auto result = vkCreateSemaphore(device->GetHandle(), &semaphoreCI, VULKAN_CPU_ALLOCATOR, &semaphore);
					if (result != VK_SUCCESS)
					{
						continue;
					}

					imageAcquiredSemaphores[i].Add(semaphore);

					VkFenceCreateInfo fenceCI{};
					fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
					fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

					VkFence fence = nullptr;
					result = vkCreateFence(device->GetHandle(), &fenceCI, VULKAN_CPU_ALLOCATOR, &fence);
					if (result != VK_SUCCESS)
					{
						continue;
					}

					imageAcquiredFences[i].Add(fence);
				}
			}
		}

		for (const auto& rhiScope : frameGraph->scopes)
		{
			Vulkan::Scope* scope = (Vulkan::Scope*)rhiScope;
			for (int imageIdx = 0; imageIdx < imageCount; imageIdx++)
			{
				scope->commandListsByFamilyIndexPerImage.Add({});

				for (u32 familyIdx = 0; familyIdx < device->queueFamilyProperties.GetSize(); familyIdx++)
				{
					VkCommandBuffer cmdBuffer = nullptr;
					VkCommandPool pool = device->AllocateCommandBuffers(1, &cmdBuffer, RHI::CommandListType::Direct, familyIdx);
					CommandList* commandList = new Vulkan::CommandList(device, cmdBuffer, RHI::CommandListType::Direct, familyIdx, pool);
					scope->commandListsByFamilyIndexPerImage[imageIdx].Add(commandList);
				}
			}
		}

		// Compile barriers
		CompileBarriers(compileRequest);

	}

    void FrameGraphCompiler::DestroySyncObjects()
    {
		for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
		{
			for (VkSemaphore semaphore : imageAcquiredSemaphores[i])
			{
				vkDestroySemaphore(device->GetHandle(), semaphore, VULKAN_CPU_ALLOCATOR);
			}
			imageAcquiredSemaphores[i].Clear();

			for (VkFence fence : imageAcquiredFences[i])
			{
				vkDestroyFence(device->GetHandle(), fence, VULKAN_CPU_ALLOCATOR);
			}
			imageAcquiredFences[i].Clear();
		}
    }

    //! If two scopes are executed one different queues and there's a dependency between them, then we
	//! need to use a wait semaphore for it.
    void FrameGraphCompiler::CompileCrossQueueDependencies(const RHI::FrameGraphCompileRequest& compileRequest)
	{
		HashSet<RHI::ScopeId> visitedScopes{};
		for (auto scope : compileRequest.frameGraph->producers)
		{
			CompileCrossQueueDependenciesInternal(compileRequest, (Vulkan::Scope*)scope, visitedScopes);
		}
	}

	void FrameGraphCompiler::CompileCrossQueueDependenciesInternal(const RHI::FrameGraphCompileRequest& compileRequest, 
		Vulkan::Scope* current, 
		HashSet<RHI::ScopeId>& visitedScopes)
	{
		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;
		if (visitedScopes.Exists(current->GetId()))
			return;

		visitedScopes.Add(current->GetId());

		const auto& producers = current->producers;

		for (RHI::Scope* rhiScope : producers)
		{
			Vulkan::Scope* producerScope = (Vulkan::Scope*)rhiScope;

			HashMap<RHI::ScopeAttachment*, RHI::ScopeAttachment*> commonAttachments = Scope::FindCommonFrameAttachments(producerScope, current);

			for (auto [from, to] : commonAttachments)
			{
				VkPipelineStageFlags flags{};

				if (to->GetUsage() == RHI::ScopeAttachmentUsage::Color)
				{
					flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
				else if (to->GetUsage() == RHI::ScopeAttachmentUsage::DepthStencil)
				{
					flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				}
				else if (to->GetUsage() == RHI::ScopeAttachmentUsage::SubpassInput || to->GetUsage() == RHI::ScopeAttachmentUsage::Shader)
				{
					flags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				}
				else
				{
					flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}

				flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

				for (int i = 0; i < imageCount; i++)
				{
					current->waitSemaphores[i].Add(producerScope->signalSemaphoresByConsumerScope[i][current->id]);
				}
				current->waitSemaphoreStageFlags.Add(flags);
			}
		}

		for (RHI::Scope* consumer : current->consumers)
		{
			CompileCrossQueueDependenciesInternal(compileRequest, (Vulkan::Scope*)consumer, visitedScopes);
		}
	}

	void FrameGraphCompiler::CompileBarriers(const RHI::FrameGraphCompileRequest& compileRequest)
	{
		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;
		for (int i = 0; i < visitedScopes.GetSize(); i++)
		{
			visitedScopes[i].Clear();
		}

		for (int i = 0; i < usedAttachments.GetSize(); i++)
		{
			usedAttachments[i].Clear();
		}

		for (int i = 0; i < imageCount; i++)
		{
			for (RHI::Scope* scope : frameGraph->producers)
			{
				CompileBarriers(i, compileRequest, (Vulkan::Scope*)scope);
			}
		}
	}

	static bool RequiresDependency(RHI::ScopeAttachment* from, RHI::ScopeAttachment* to)
	{
		if (from == nullptr || to == nullptr)
			return false;

		if (EnumHasFlag(to->GetAccess(), RHI::ScopeAttachmentAccess::Write) ||
			EnumHasFlag(from->GetAccess(), RHI::ScopeAttachmentAccess::Write))
			return true;

		switch (to->GetUsage())
		{
		case RHI::ScopeAttachmentUsage::Color:
		case RHI::ScopeAttachmentUsage::Resolve:
			return true;
		}

		switch (from->GetUsage())
		{
		case RHI::ScopeAttachmentUsage::Color:
		case RHI::ScopeAttachmentUsage::Resolve:
			return true;
		}

		return false;
	}

	void FrameGraphCompiler::CompileBarriers(int imageIndex, const RHI::FrameGraphCompileRequest& compileRequest, Vulkan::Scope* current)
	{
		if (current == nullptr)
			return;

		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;

		if (!visitedScopes[imageIndex].Exists(current->id))
		{
			current->initialBarriers[imageIndex].Clear();
			current->barriers[imageIndex].Clear();

			for (RHI::Scope* producerRhiScope : current->producers)
			{
				Vulkan::Scope* producerScope = (Vulkan::Scope*)producerRhiScope;

				bool isDifferentQueue = producerScope->queue != current->queue;

				HashMap<RHI::ScopeAttachment*, RHI::ScopeAttachment*> commonAttachments = Scope::FindCommonFrameAttachments(producerRhiScope, current);

				// TODO: Add support for Compute Shader barriers
				// Currently, Shader attachments only consider Vertex/Fragment shaders
				// We can use producerScope and current to determine which one is a Raster pass or a compute pass

				for (auto [from, to] : commonAttachments)
				{
					Scope::Barrier barrier{};

					// Image -> Image barrier
					if (from->IsImageAttachment() && to->IsImageAttachment() && RequiresDependency(from, to))
					{
						RHI::ImageScopeAttachment* fromImage = (RHI::ImageScopeAttachment*)from;
						RHI::ImageScopeAttachment* toImage = (RHI::ImageScopeAttachment*)to;
						VkImageMemoryBarrier imageBarrier{};
						imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

						RHI::ImageFrameAttachment* imageAttachment = (RHI::ImageFrameAttachment*)fromImage->GetFrameAttachment();
						if (imageAttachment == nullptr)
							continue;

						RHI::RHIResource* resource = imageAttachment->GetResource(imageIndex);
						if (resource == nullptr)
							continue;

						Texture* image = nullptr;

						if (resource->GetResourceType() == RHI::ResourceType::Texture)
						{
							image = dynamic_cast<Texture*>(resource);
						}
						else if (resource->GetResourceType() == RHI::ResourceType::TextureView)
						{
							TextureView* imageView = dynamic_cast<TextureView*>(resource);
							image = (Texture*)imageView->GetTexture();
						}
						else
						{
							continue;
						}

						if (image == nullptr || image->GetImage() == nullptr)
							continue;

						imageBarrier.image = image->GetImage();
						imageBarrier.srcQueueFamilyIndex = producerScope->queue->GetFamilyIndex();
						imageBarrier.dstQueueFamilyIndex = current->queue->GetFamilyIndex();

						imageBarrier.subresourceRange.aspectMask = image->GetAspectMask();
						imageBarrier.subresourceRange.baseArrayLayer = 0;
						imageBarrier.subresourceRange.layerCount = image->GetArrayLayerCount();
						imageBarrier.subresourceRange.baseMipLevel = 0;
						imageBarrier.subresourceRange.levelCount = image->GetMipLevelCount();

						switch (fromImage->GetUsage())
						{
						case RHI::ScopeAttachmentUsage::Color:
							barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							break;
						case RHI::ScopeAttachmentUsage::DepthStencil:
							barrier.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							imageBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
							imageBarrier.srcAccessMask = 0;
							if (EnumHasFlag(fromImage->GetAccess(), RHI::ScopeAttachmentAccess::Write))
							{
								imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							}
							else // Read only
							{
								imageBarrier.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
								imageBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
							}
							break;
						case RHI::ScopeAttachmentUsage::SubpassInput:
						case RHI::ScopeAttachmentUsage::Shader:
							barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
							imageBarrier.srcAccessMask = 0;
							if (EnumHasFlag(fromImage->GetAccess(), RHI::ScopeAttachmentAccess::Write))
							{
								imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
								imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL; // You can only write to general layouts in shader
							}
							else // Read only
							{
								imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
								if (producerScope->GetOperation() == RHI::ScopeOperation::Compute)
									imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
								else
									imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							}
							break;
						case RHI::ScopeAttachmentUsage::Copy:
							barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
							imageBarrier.srcAccessMask = 0;
							if (EnumHasFlag(fromImage->GetAccess(), RHI::ScopeAttachmentAccess::Read))
							{
								imageBarrier.srcAccessMask |= VK_ACCESS_MEMORY_READ_BIT;
								imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
							}
							else if (EnumHasFlag(fromImage->GetAccess(), RHI::ScopeAttachmentAccess::Write))
							{
								imageBarrier.srcAccessMask |= VK_ACCESS_MEMORY_WRITE_BIT;
								imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
							}
							break;
						case RHI::ScopeAttachmentUsage::Resolve:
							barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							break;
						default:
							continue;
						}

						switch (toImage->GetUsage())
						{
						case RHI::ScopeAttachmentUsage::Color:
							barrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							break;
						case RHI::ScopeAttachmentUsage::DepthStencil:
							barrier.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
							imageBarrier.dstAccessMask = 0;
							if (EnumHasFlag(toImage->GetAccess(), RHI::ScopeAttachmentAccess::Write))
							{
								imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							}
							else // Read only
							{
								imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
								imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
							}
							break;
						case RHI::ScopeAttachmentUsage::SubpassInput:
						case RHI::ScopeAttachmentUsage::Shader:
							barrier.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
							imageBarrier.dstAccessMask = 0;
							if (EnumHasFlag(toImage->GetAccess(), RHI::ScopeAttachmentAccess::Write))
							{
								imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
								imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
							}
							else // Read only
							{
								imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
								if (current->GetOperation() == RHI::ScopeOperation::Compute)
									imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
								else
									imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							}
							break;
						case RHI::ScopeAttachmentUsage::Copy:
							barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
							imageBarrier.dstAccessMask = 0;
							if (EnumHasFlag(fromImage->GetAccess(), RHI::ScopeAttachmentAccess::Read))
							{
								imageBarrier.dstAccessMask |= VK_ACCESS_MEMORY_READ_BIT;
								imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
							}
							else if (EnumHasFlag(fromImage->GetAccess(), RHI::ScopeAttachmentAccess::Write))
							{
								imageBarrier.dstAccessMask |= VK_ACCESS_MEMORY_WRITE_BIT;
								imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
							}
							break;
						case RHI::ScopeAttachmentUsage::Resolve:
							barrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							break;
						default:
							continue;
						}

						VkPipelineStageFlags originalDstStageMask = barrier.dstStageMask;
						VkAccessFlags originalDstAccessMask = imageBarrier.dstAccessMask;

						if (isDifferentQueue)
						{
							barrier.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
							imageBarrier.dstAccessMask = 0;
						}

						Scope::ImageLayoutTransition transition{};
						transition.image = image;
						transition.layout = imageBarrier.newLayout;
						transition.queueFamilyIndex = current->queue->GetFamilyIndex();

						barrier.imageBarriers.Add(imageBarrier);
						barrier.imageLayoutTransitions.Add(transition);
						
						producerScope->barriers[imageIndex].Add(barrier);

						if (isDifferentQueue)
						{
							barrier.imageBarriers.Clear();
							barrier.dstStageMask = originalDstStageMask;
							imageBarrier.dstAccessMask = originalDstAccessMask;
							barrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
							imageBarrier.srcAccessMask = 0;

							barrier.imageBarriers.Add(imageBarrier);

							current->initialBarriers[imageIndex].Add(barrier);
						}
					}
					// Buffer -> Buffer barrier
					else if (from->IsBufferAttachment() && to->IsBufferAttachment() && RequiresDependency(from, to))
					{
						RHI::BufferScopeAttachment* fromBuffer = (RHI::BufferScopeAttachment*)from;
						RHI::BufferScopeAttachment* toBuffer = (RHI::BufferScopeAttachment*)to;
						VkBufferMemoryBarrier bufferBarrier{};
						bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

						RHI::BufferFrameAttachment* bufferAttachment = (RHI::BufferFrameAttachment*)fromBuffer->GetFrameAttachment();
						if (bufferAttachment == nullptr)
							continue;

						RHI::RHIResource* resource = bufferAttachment->GetResource(imageIndex);
						if (resource == nullptr || resource->GetResourceType() != RHI::ResourceType::Buffer)
							continue;

						Vulkan::Buffer* buffer = dynamic_cast<Vulkan::Buffer*>(resource);
						if (buffer == nullptr || buffer->GetBuffer() == nullptr)
							continue;

						bufferBarrier.buffer = buffer->GetBuffer();
						bufferBarrier.srcQueueFamilyIndex = producerScope->queue->GetFamilyIndex();
						bufferBarrier.dstQueueFamilyIndex = current->queue->GetFamilyIndex();
						bufferBarrier.offset = 0;
						bufferBarrier.size = buffer->GetBufferSize();

						switch (fromBuffer->GetUsage())
						{
						case RHI::ScopeAttachmentUsage::Copy:
						case RHI::ScopeAttachmentUsage::Shader:
							if (EnumHasFlag(fromBuffer->GetAccess(), RHI::ScopeAttachmentAccess::Read))
							{
								bufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
							}
							else if (EnumHasFlag(fromBuffer->GetAccess(), RHI::ScopeAttachmentAccess::Write))
							{
								bufferBarrier.srcAccessMask |= VK_ACCESS_MEMORY_WRITE_BIT;
							}
							break;
						default:
							continue;
						}

						switch (toBuffer->GetUsage())
						{
						case RHI::ScopeAttachmentUsage::Copy:
						case RHI::ScopeAttachmentUsage::Shader:
							if (EnumHasFlag(toBuffer->GetAccess(), RHI::ScopeAttachmentAccess::Read))
							{
								bufferBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
							}
							else if (EnumHasFlag(toBuffer->GetAccess(), RHI::ScopeAttachmentAccess::Write))
							{
								bufferBarrier.dstAccessMask |= VK_ACCESS_MEMORY_WRITE_BIT;
							}
							break;
						default:
							continue;
						}

						Scope::BufferFamilyTransition transition{};
						transition.buffer = buffer;
						transition.queueFamilyIndex = current->queue->GetFamilyIndex();

						barrier.bufferBarriers.Add(bufferBarrier);
						barrier.bufferFamilyTransitions.Add(transition);

						producerScope->barriers[imageIndex].Add(barrier);
					}
					
				}
			}
		}

		visitedScopes[imageIndex].Add(current->id);

		for (auto scope : current->consumers)
		{
			CompileBarriers(imageIndex, compileRequest, (Vulkan::Scope*)scope);
		}
	}

} // namespace CE::Vulkan
