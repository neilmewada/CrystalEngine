#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    
	FrameGraphCompiler::FrameGraphCompiler(Device* device) : device(device)
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

		// Queue allocation logic

		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;

		CommandQueue* graphicsQueue = device->GetGraphicsQueue();
		CommandQueue* computeQueue = graphicsQueue;

		for (int t = 0; t <= frameGraph->maxTimelineLevel; t++)
		{
			ArrayView scopes = frameGraph->scopesByTimelineLevel[t];
			
			for (int i = 0; i < scopes.GetSize(); i++)
			{
				auto scope = scopes[i];

				bool useComputeQueue = scopes.GetSize() > 1 && scopes[i]->IsComputePass();
				((Vulkan::Scope*)scopes[i])->queue = useComputeQueue ? computeQueue : graphicsQueue;

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
			}
		}
	}

	void FrameGraphCompiler::CompileInternal(const RHI::FrameGraphCompileRequest& compileRequest)
	{
		ZoneScoped;

		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;

		const u32 numSwapChains = frameGraph->presentSwapChains.GetSize();
		const u32 frameSlot = compileRequest.frameSlot;
		numFramesInFlight = RHI::Limits::MaxFramesInFlight;

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

		CompileCrossQueueDependencies(compileRequest);

		CompileBarriers(compileRequest);
	}

	void FrameGraphCompiler::DestroySyncObjects()
    {
		
    }

    // If two scopes are executed one different queues and there's a cross queue dependency.
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
		ZoneScoped;

		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;
		if (visitedScopes.Exists(current->GetId()))
			return;

		visitedScopes.Add(current->GetId());

		const auto& producers = current->producers;

		for (RHI::Scope* rhiScope : producers)
		{
			Vulkan::Scope* producerScope = (Vulkan::Scope*)rhiScope;

			if (producerScope->queue == current->queue)
				continue;

			HashMap<RHI::ScopeAttachment*, RHI::ScopeAttachment*> commonAttachments = Scope::FindCommonFrameAttachments(producerScope, current);

			VkPipelineStageFlags flags = 0;

			for (auto [from, to] : commonAttachments)
			{
				if (to->GetUsage() == RHI::ScopeAttachmentUsage::Color)
				{
					flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
				else if (to->GetUsage() == RHI::ScopeAttachmentUsage::DepthStencil)
				{
					flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				}
				else if (to->GetUsage() == RHI::ScopeAttachmentUsage::SubpassInput || to->GetUsage() == RHI::ScopeAttachmentUsage::Shader)
				{
					flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				}
				else
				{
					flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				}
			}

		}

		for (RHI::Scope* consumer : current->consumers)
		{
			CompileCrossQueueDependenciesInternal(compileRequest, (Vulkan::Scope*)consumer, visitedScopes);
		}
	}

	void FrameGraphCompiler::CompileBarriers(const RHI::FrameGraphCompileRequest& compileRequest)
	{
		ZoneScoped;

		RHI::FrameGraph* frameGraph = compileRequest.frameGraph;
		executionPlan.submissions.Clear();

		if (frameGraph == nullptr || frameGraph->GetTopologicallySortedScopes().IsEmpty())
			return;

		struct ImageState
		{
			VkPipelineStageFlags2 stages = VK_PIPELINE_STAGE_2_NONE;
			VkAccessFlags2 access = VK_ACCESS_2_NONE;
			VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
			bool initialized = false;
		};

		struct BufferState
		{
			VkPipelineStageFlags2 stages = VK_PIPELINE_STAGE_2_NONE;
			VkAccessFlags2 access = VK_ACCESS_2_NONE;
			bool initialized = false;
		};

		auto getShaderStages = [](const Vulkan::Scope* scope)
		{
			if (scope->IsComputePass())
				return VkPipelineStageFlags2(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);
			if (scope->IsGraphicsPass())
				return VkPipelineStageFlags2(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
			return VkPipelineStageFlags2(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
		};

		auto getImageState = [&getShaderStages](const Vulkan::Scope* scope, const RHI::ScopeAttachment* attachment)
		{
			ImageState state{};
			const bool reads = EnumHasFlag(attachment->GetAccess(), RHI::ScopeAttachmentAccess::Read);
			const bool writes = EnumHasFlag(attachment->GetAccess(), RHI::ScopeAttachmentAccess::Write);

			switch (attachment->GetUsage())
			{
			case RHI::ScopeAttachmentUsage::Color:
			case RHI::ScopeAttachmentUsage::Resolve:
				state.stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
				state.access = (reads ? VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT : 0) |
					(writes ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT : 0);
				state.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case RHI::ScopeAttachmentUsage::DepthStencil:
				state.stages = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
				state.access = (reads ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT : 0) |
					(writes ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0);
				state.layout = writes ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				break;
			case RHI::ScopeAttachmentUsage::Copy:
				state.stages = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				state.access = (reads ? VK_ACCESS_2_TRANSFER_READ_BIT : 0) |
					(writes ? VK_ACCESS_2_TRANSFER_WRITE_BIT : 0);
				state.layout = reads && writes ? VK_IMAGE_LAYOUT_GENERAL :
					(reads ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				break;
			case RHI::ScopeAttachmentUsage::Shader:
				state.stages = getShaderStages(scope);
				state.access = (reads ? VK_ACCESS_2_SHADER_READ_BIT : 0) |
					(writes ? VK_ACCESS_2_SHADER_WRITE_BIT : 0);
				state.layout = writes ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				break;
			case RHI::ScopeAttachmentUsage::SubpassInput:
				state.stages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
				state.access = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
				state.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				break;
			default:
				return state;
			}

			state.initialized = true;
			return state;
		};

		auto getBufferState = [&getShaderStages](const Vulkan::Scope* scope, const RHI::ScopeAttachment* attachment)
		{
			BufferState state{};
			const bool reads = EnumHasFlag(attachment->GetAccess(), RHI::ScopeAttachmentAccess::Read);
			const bool writes = EnumHasFlag(attachment->GetAccess(), RHI::ScopeAttachmentAccess::Write);

			switch (attachment->GetUsage())
			{
			case RHI::ScopeAttachmentUsage::Copy:
				state.stages = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				state.access = (reads ? VK_ACCESS_2_TRANSFER_READ_BIT : 0) |
					(writes ? VK_ACCESS_2_TRANSFER_WRITE_BIT : 0);
				break;
			case RHI::ScopeAttachmentUsage::Shader:
				state.stages = getShaderStages(scope);
				state.access = (reads ? VK_ACCESS_2_SHADER_READ_BIT : 0) |
					(writes ? VK_ACCESS_2_SHADER_WRITE_BIT : 0);
				break;
			default:
				return state;
			}

			state.initialized = true;
			return state;
		};

		auto hasWriteAccess = [](VkAccessFlags2 access)
		{
			constexpr VkAccessFlags2 writeAccess = VK_ACCESS_2_MEMORY_WRITE_BIT |
				VK_ACCESS_2_SHADER_WRITE_BIT |
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_2_TRANSFER_WRITE_BIT;
			return (access & writeAccess) != 0;
		};

		HashMap<RHI::ImageFrameAttachment*, ImageState> imageStates{};
		HashMap<RHI::BufferFrameAttachment*, BufferState> bufferStates{};

		executionPlan.submissions.Add({});
		Submission& submission = executionPlan.submissions[0];
		submission.queue = device->GetGraphicsQueue();

		for (RHI::Scope* rhiScope : frameGraph->GetTopologicallySortedScopes())
		{
			Vulkan::Scope* scope = (Vulkan::Scope*)rhiScope;
			ExecutionStep step{};
			step.scope = scope;

			for (RHI::ScopeAttachment* scopeAttachment : scope->GetAttachments())
			{
				RHI::FrameAttachment* frameAttachment = scopeAttachment->GetFrameAttachment();
				if (frameAttachment == nullptr)
					continue;

				if (scopeAttachment->IsImageAttachment() && frameAttachment->IsImageAttachment())
				{
					auto imageAttachment = (RHI::ImageFrameAttachment*)frameAttachment;
					ImageState required = getImageState(scope, scopeAttachment);
					if (!required.initialized)
						continue;

					const bool isFirstUse = !imageStates.KeyExists(imageAttachment);
					ImageState previous{};
					if (!isFirstUse)
					{
						previous = imageStates[imageAttachment];
					}
					else
					{
						previous.initialized = true;
						previous.stages = imageAttachment->IsSwapChainAttachment() ?
							VK_PIPELINE_STAGE_2_NONE : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
						previous.access = imageAttachment->IsSwapChainAttachment() ?
							VK_ACCESS_2_NONE : VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
						previous.layout = imageAttachment->IsSwapChainAttachment() ?
							VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_UNDEFINED;
					}

					const bool needsBarrier = previous.layout != required.layout ||
						hasWriteAccess(previous.access) || hasWriteAccess(required.access);

					if (needsBarrier)
					{
						const RHI::ImageDescriptor& descriptor = imageAttachment->GetImageDescriptor();
						VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						if (IsDepthStencilFormat(descriptor.format))
							aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
						else if (IsDepthFormat(descriptor.format))
							aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

						ImageBarrier compiledBarrier{};
						compiledBarrier.attachment = imageAttachment;
						compiledBarrier.resolveOldLayoutFromResource = isFirstUse;
						compiledBarrier.barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
						compiledBarrier.barrier.srcStageMask = previous.stages;
						compiledBarrier.barrier.srcAccessMask = previous.access;
						compiledBarrier.barrier.dstStageMask = required.stages;
						compiledBarrier.barrier.dstAccessMask = required.access;
						compiledBarrier.barrier.oldLayout = previous.layout;
						compiledBarrier.barrier.newLayout = required.layout;
						compiledBarrier.barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						compiledBarrier.barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						compiledBarrier.barrier.image = VK_NULL_HANDLE;
						compiledBarrier.barrier.subresourceRange.aspectMask = aspectMask;
						compiledBarrier.barrier.subresourceRange.baseMipLevel = 0;
						compiledBarrier.barrier.subresourceRange.levelCount = descriptor.mipLevels;
						compiledBarrier.barrier.subresourceRange.baseArrayLayer = 0;
						compiledBarrier.barrier.subresourceRange.layerCount = descriptor.arrayLayers;

						step.preBarriers.imageBarriers.Add(compiledBarrier);
						step.preBarriers.imageLayoutTransitions.Add({ imageAttachment, required.layout,
							submission.queue->GetFamilyIndex() });
					}

					imageStates[imageAttachment] = required;
				}
				else if (scopeAttachment->IsBufferAttachment() && frameAttachment->IsBufferAttachment())
				{
					auto bufferAttachment = (RHI::BufferFrameAttachment*)frameAttachment;
					BufferState required = getBufferState(scope, scopeAttachment);
					if (!required.initialized)
						continue;

					BufferState previous{};
					if (bufferStates.KeyExists(bufferAttachment))
					{
						previous = bufferStates[bufferAttachment];
					}
					else
					{
						previous.initialized = true;
						previous.stages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
						previous.access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
					}

					if (hasWriteAccess(previous.access) || hasWriteAccess(required.access))
					{
						BufferBarrier compiledBarrier{};
						compiledBarrier.attachment = bufferAttachment;
						compiledBarrier.barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
						compiledBarrier.barrier.srcStageMask = previous.stages;
						compiledBarrier.barrier.srcAccessMask = previous.access;
						compiledBarrier.barrier.dstStageMask = required.stages;
						compiledBarrier.barrier.dstAccessMask = required.access;
						compiledBarrier.barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						compiledBarrier.barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						compiledBarrier.barrier.buffer = VK_NULL_HANDLE;
						compiledBarrier.barrier.offset = 0;
						compiledBarrier.barrier.size = VK_WHOLE_SIZE;

						step.preBarriers.bufferBarriers.Add(compiledBarrier);
						step.preBarriers.bufferFamilyTransitions.Add({ bufferAttachment,
							submission.queue->GetFamilyIndex() });
					}

					bufferStates[bufferAttachment] = required;
				}
			}

			if (scope->PresentsSwapChain())
			{
				for (RHI::ScopeAttachment* scopeAttachment : scope->GetAttachments())
				{
					RHI::FrameAttachment* frameAttachment = scopeAttachment->GetFrameAttachment();
					if (frameAttachment == nullptr || !frameAttachment->IsSwapChainAttachment())
						continue;

					auto imageAttachment = (RHI::ImageFrameAttachment*)frameAttachment;
					if (!imageStates.KeyExists(imageAttachment))
						continue;

					ImageState previous = imageStates[imageAttachment];
					if (previous.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
						continue;

					ImageBarrier compiledBarrier{};
					compiledBarrier.attachment = imageAttachment;
					compiledBarrier.barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
					compiledBarrier.barrier.srcStageMask = previous.stages;
					compiledBarrier.barrier.srcAccessMask = previous.access;
					compiledBarrier.barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
					compiledBarrier.barrier.dstAccessMask = VK_ACCESS_2_NONE;
					compiledBarrier.barrier.oldLayout = previous.layout;
					compiledBarrier.barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					compiledBarrier.barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					compiledBarrier.barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					compiledBarrier.barrier.image = VK_NULL_HANDLE;
					compiledBarrier.barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					compiledBarrier.barrier.subresourceRange.baseMipLevel = 0;
					compiledBarrier.barrier.subresourceRange.levelCount = 1;
					compiledBarrier.barrier.subresourceRange.baseArrayLayer = 0;
					compiledBarrier.barrier.subresourceRange.layerCount = 1;

					step.postBarriers.imageBarriers.Add(compiledBarrier);
					step.postBarriers.imageLayoutTransitions.Add({ imageAttachment,
						VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, submission.queue->GetFamilyIndex() });

					previous.stages = VK_PIPELINE_STAGE_2_NONE;
					previous.access = VK_ACCESS_2_NONE;
					previous.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					imageStates[imageAttachment] = previous;
				}
			}

			submission.steps.Add(step);
		}
	}

} // namespace CE::Vulkan
