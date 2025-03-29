#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

	FrameGraphExecuter::FrameGraphExecuter(VulkanDevice* device) : device(device)
	{
		
	}

	FrameGraphExecuter::~FrameGraphExecuter()
	{
		device->GetShaderResourceManager()->DestroyQueuedSRG();
	}

	bool FrameGraphExecuter::ExecuteInternal(const FrameGraphExecuteRequest& executeRequest)
	{
		ZoneScoped;

		device->GetShaderResourceManager()->DestroyQueuedSRG();

		FrameGraph* frameGraph = executeRequest.frameGraph;
		compiler = (Vulkan::FrameGraphCompiler*)executeRequest.compiler;
		//Vulkan::Scope* presentingScope = (Vulkan::Scope*)frameGraph->presentingScope;
		//auto swapChain = (Vulkan::SwapChain*)frameGraph->presentSwapChain;
		bool swapChainExists = frameGraph->presentSwapChains.NotEmpty();

		const Array<RHI::Scope*>& producers = frameGraph->producers;
		constexpr u64 u64Max = NumericLimits<u64>::Max();
		VkResult result = VK_SUCCESS;
		//currentImageIndices.Clear();

		if (swapChainExists)
		{
            vkResetFences(device->GetHandle(), compiler->imageAcquiredFences[currentSubmissionIndex].GetSize(), 
				compiler->imageAcquiredFences[currentSubmissionIndex].GetData());
            
			for (int i = 0; i < frameGraph->presentSwapChains.GetSize(); i++)
			{
				Vulkan::SwapChain* swapChain = (Vulkan::SwapChain*)frameGraph->presentSwapChains[i];

				result = vkAcquireNextImageKHR(device->GetHandle(), 
					swapChain->GetHandle(), u64Max,
					compiler->imageAcquiredSemaphores[currentSubmissionIndex][i],
					//compiler->imageAcquiredFences[currentSubmissionIndex],
					nullptr,
					&swapChain->currentImageIndex);

				if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
				{
					for (auto swapChainToRebuild : frameGraph->presentSwapChains)
					{
						((Vulkan::SwapChain*)swapChainToRebuild)->RebuildSwapChain();
					}
					return false; // TODO: Try acquiring image again next frame...
				}

				if (result != VK_SUCCESS)
				{
					return false;
				}

				if (i == 0)
				{
					// TODO: Switch to using currentSubmissionIndex instead
					//currentImageIndex = swapChain->currentImageIndex;
				}
				//currentImageIndices.Add(swapChain->currentImageIndex);
			}
			
			vkWaitForFences(device->GetHandle(),
				compiler->graphExecutionFences[currentSubmissionIndex].GetSize(),
				compiler->graphExecutionFences[currentSubmissionIndex].GetData(),
				VK_TRUE, u64Max);

		}
		else
		{
			//currentImageIndex = currentSubmissionIndex;
			//currentImageIndices.Add(currentImageIndex);
		}

		bool success = true;

		HashSet<ScopeId> executedScopes{};
		HashSet<Vulkan::SwapChain*> usedSwapChains{};

		for (auto rhiScope : frameGraph->endScopes)
		{
			ExecuteScope(executeRequest, (Vulkan::Scope*)rhiScope, executedScopes, usedSwapChains);
		}

		currentSubmissionIndex = (currentSubmissionIndex + 1) % compiler->imageCount;
		totalFramesSubmitted++;

		return success && result == VK_SUCCESS;
	}

	void FrameGraphExecuter::WaitUntilIdle()
	{
		ZoneScoped;

		vkDeviceWaitIdle(device->GetHandle());
		return;

		constexpr u64 u64Max = NumericLimits<u64>::Max();

		for (int i = 0; i < compiler->graphExecutionFences.GetSize(); ++i)
		{
			vkWaitForFences(device->GetHandle(),
				compiler->graphExecutionFences[i].GetSize(),
				compiler->graphExecutionFences[i].GetData(),
				VK_TRUE, u64Max);
		}
	}

	u32 FrameGraphExecuter::BeginExecution(const RHI::FrameGraphExecuteRequest& executeRequest)
	{
		ZoneScoped;

		device->GetShaderResourceManager()->DestroyQueuedSRG();

		RHI::FrameGraph* frameGraph = executeRequest.frameGraph;
		compiler = (Vulkan::FrameGraphCompiler*)executeRequest.compiler;
		bool swapChainExists = frameGraph->presentSwapChains.NotEmpty();

		const Array<RHI::Scope*>& producers = frameGraph->producers;
		constexpr u64 u64Max = NumericLimits<u64>::Max();
        constexpr u64 acquireTimeout = 100'000'000; // 0.1 second
		VkResult result = VK_SUCCESS;

		if (swapChainExists)
		{
			vkResetFences(device->GetHandle(),
				compiler->imageAcquiredFences[currentSubmissionIndex].GetSize(),
				compiler->imageAcquiredFences[currentSubmissionIndex].GetData());

			{
				ZoneNamedN(__graphExecution, "_GraphExecutionFences", true);
				String val = String("Index: ") + currentSubmissionIndex;
				ZoneText(val.GetCString(), val.GetLength());

				vkWaitForFences(device->GetHandle(),
				   compiler->graphExecutionFences[currentSubmissionIndex].GetSize(),
				   compiler->graphExecutionFences[currentSubmissionIndex].GetData(),
				   VK_TRUE, u64Max);
			}

			for (int i = 0; i < frameGraph->presentSwapChains.GetSize(); i++)
			{
				auto swapChain = (Vulkan::SwapChain*)frameGraph->presentSwapChains[i];

				{
					ZoneNamedN(__acquireImage, "_AcquireNextImage", true);

					result = vkAcquireNextImageKHR(device->GetHandle(),
					   swapChain->GetHandle(), acquireTimeout,
					   compiler->imageAcquiredSemaphores[currentSubmissionIndex][i],
					   //compiler->imageAcquiredFences[currentSubmissionIndex][i],
					   nullptr,
					   &swapChain->currentImageIndex);
				}

				if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
				{
					for (auto swapChainToRebuild : frameGraph->presentSwapChains)
					{
						ZoneNamedN(__rebuildSwapChain, "_RebuildSwapChain", true);

						((Vulkan::SwapChain*)swapChain)->RebuildSwapChain();
					}
				}

				if (result != VK_SUCCESS)
				{
					return RHI::Limits::MaxSwapChainImageCount;
				}
			}
		}

		return currentSubmissionIndex;
	}

	void FrameGraphExecuter::EndExecution(const RHI::FrameGraphExecuteRequest& executeRequest)
	{
		ZoneScoped;
		String value = String("Index: ") + currentSubmissionIndex;
		ZoneText(value.GetCString(), value.GetLength());

		RHI::FrameGraph* frameGraph = executeRequest.frameGraph;

		HashSet<RHI::ScopeId> executedScopes{};
		HashSet<Vulkan::SwapChain*> usedSwapChains{};

		for (auto rhiScope : frameGraph->endScopes)
		{
			ExecuteScope(executeRequest, (Vulkan::Scope*)rhiScope, executedScopes, usedSwapChains);
		}

		currentSubmissionIndex = (currentSubmissionIndex + 1) % compiler->imageCount;
		totalFramesSubmitted++;
	}

	void FrameGraphExecuter::ResetFramesInFlight()
	{
		currentSubmissionIndex = 0;
	}

	bool FrameGraphExecuter::ExecuteScope(const RHI::FrameGraphExecuteRequest& executeRequest, Vulkan::Scope* scope, 
	                                      HashSet<RHI::ScopeId>& executedScopes,
	                                      HashSet<Vulkan::SwapChain*>& usedSwapChains)
	{
		if (!scope)
			return false;

		ZoneScoped;

		for (auto rhiProducer : scope->producers)
		{
			ExecuteScope(executeRequest, (Vulkan::Scope*)rhiProducer, executedScopes, usedSwapChains);
		}

		if (executedScopes.Exists(scope->id))
			return false;
		if (scope->IsSubPass() && scope->prevSubPass != nullptr)
			return false;

		RHI::FrameGraph* frameGraph = executeRequest.frameGraph;
		RHI::FrameScheduler* scheduler = executeRequest.scheduler;
		FrameGraphCompiler* compiler = (Vulkan::FrameGraphCompiler*)executeRequest.compiler;

		bool presentRequired = false;
		auto presentQueue = device->GetPresentQueue();

		Array<RHI::SwapChain*> presentSwapChains = frameGraph->presentSwapChains;
		
		const RHI::FrameGraph::GraphNode& graphNode = frameGraph->nodes[scope->id];
		
		constexpr u64 u64Max = NumericLimits<u64>::Max();
		VkResult result = VK_SUCCESS;

		// Wait for rendering from earlier submission to finish.
		// We cannot record new commands into a command buffer that is currently being executed.
		result = vkWaitForFences(device->GetHandle(), 1, &scope->renderFinishedFences[currentSubmissionIndex], VK_TRUE, u64Max);
		
		u32 familyIndex = scope->queue->GetFamilyIndex();
		CommandList* commandList = scope->commandListsByFamilyIndexPerImage[currentSubmissionIndex][familyIndex];

		Array<RHI::Scope*> consumers{};

		Array<Scope*> scopeChain{};
		Scope* scopeInChain = scope;
		HashSet<Vulkan::SwapChain*> swapChainsUsedAsAttachmentForFirstTime{};

		while (scopeInChain != nullptr)
		{
			if (scopeInChain->swapChainsUsedByAttachments.NotEmpty())
			{
				for (auto swapChain : scopeInChain->swapChainsUsedByAttachments)
				{
					swapChainsUsedAsAttachmentForFirstTime.Add((Vulkan::SwapChain*)swapChain);
				}
				//isFirstSwapChainUse = true;
			}
			scopeChain.Add(scopeInChain);
			scopeInChain = (Vulkan::Scope*)scopeInChain->next;
		}

		for (const auto& executedScopeId : executedScopes)
		{
			Vulkan::Scope* executedScope = (Vulkan::Scope*)frameGraph->scopesById[executedScopeId];
			for (auto usedSwapChain : usedSwapChains)
			{
				if (swapChainsUsedAsAttachmentForFirstTime.Exists(usedSwapChain))
				{
					swapChainsUsedAsAttachmentForFirstTime.Remove(usedSwapChain);
				}
			}
		}

		for (auto swapChainUsed : swapChainsUsedAsAttachmentForFirstTime)
		{
			usedSwapChains.Add(swapChainUsed);
		}

		if (scope->consumers.GetSize() > 1)
		{
			consumers = scope->consumers;
		}
		else
		{
			consumers = scopeChain.Top()->consumers;
		}

		if (scopeChain.Top()->PresentsSwapChain())
		{
			presentRequired = true;
		}

		auto cmdBuffer = commandList->commandBuffer;

		commandList->Begin();
		{
			commandList->SetCurrentImageIndex(currentSubmissionIndex);

			for (int scopeIndex = 0; scopeIndex < scopeChain.GetSize(); scopeIndex++)
			{
				Vulkan::Scope* currentScope = scopeChain[scopeIndex];
				if (currentScope == nullptr)
					continue;
				if (currentScope->IsComputePass())
				{
					String::IsAlphabet('a');
				}

				executedScopes.Add(currentScope->id);

				commandList->currentPass = currentScope->renderPass;
				commandList->currentSubpass = currentScope->subpassIndex;

				//bool usesSwapChainAttachment = currentScope->swapChainsUsedByAttachments.NotEmpty();
				RenderPass* renderPass = currentScope->renderPass;
				FixedArray<VkClearValue, RHI::Limits::Pipeline::MaxRenderAttachmentCount> clearValues{};
				HashSet<RHI::AttachmentID> clearedAttachments{};

				Vulkan::Scope* scopeLoop = currentScope;
				while (scopeLoop != nullptr)
				{
					for (auto scopeAttachment : scopeLoop->attachments)
					{
						if (!scopeAttachment->IsImageAttachment() || scopeAttachment->GetFrameAttachment() == nullptr ||
							!scopeAttachment->GetFrameAttachment()->IsImageAttachment())
							continue;
						if (scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Shader ||
							scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::Copy)
							continue;
						if (clearedAttachments.Exists(scopeAttachment->GetFrameAttachment()->GetId()))
							continue;

						RHI::ImageScopeAttachment* imageScopeAttachment = (RHI::ImageScopeAttachment*)scopeAttachment;
						RHI::ImageFrameAttachment* imageFrameAttachment = (RHI::ImageFrameAttachment*)scopeAttachment->GetFrameAttachment();

						VkClearValue clearValue{};

						if (scopeAttachment->GetUsage() == RHI::ScopeAttachmentUsage::DepthStencil)
						{
							clearValue.depthStencil.depth = scopeAttachment->GetLoadStoreAction().clearValueDepth;
							clearValue.depthStencil.stencil = scopeAttachment->GetLoadStoreAction().clearValueStencil;
						}
						else
						{
							const RHI::AttachmentLoadStoreAction& loadStoreAction = scopeAttachment->GetLoadStoreAction();
							memcpy(clearValue.color.float32, loadStoreAction.clearValue.xyzw, sizeof(f32[4]));
						}

						clearedAttachments.Add(scopeAttachment->GetFrameAttachment()->GetId());

						clearValues.Add(clearValue);
					}

					scopeLoop = (Vulkan::Scope*)scopeLoop->nextSubPass;
				}

				// Execute compiled pipeline barriers (initial barriers)
				if (currentScope->initialBarriers[currentSubmissionIndex].NotEmpty())
				{
					for (const auto& barrier : currentScope->initialBarriers[currentSubmissionIndex])
					{
						vkCmdPipelineBarrier(cmdBuffer,
							barrier.srcStageMask, barrier.dstStageMask,
							0,
							barrier.memoryBarriers.GetSize(), barrier.memoryBarriers.GetData(),
							barrier.bufferBarriers.GetSize(), barrier.bufferBarriers.GetData(),
							barrier.imageBarriers.GetSize(), barrier.imageBarriers.GetData());

						for (const auto& transition : barrier.imageLayoutTransitions)
						{
							transition.image->curImageLayout = transition.layout;
							transition.image->curFamilyIndex = transition.queueFamilyIndex;
						}

						for (const auto& bufferTransition : barrier.bufferFamilyTransitions)
						{
							bufferTransition.buffer->curFamilyIndex = bufferTransition.queueFamilyIndex;
						}
					}
				}

				// Additional pipeline barriers if required, for VkImageLayout and/or queue family ownership transition
				// This is 'usually' only required for first-time use of an attachment
				// The FrameGraph handles the internal resource transitions through compiled pipeline barriers.
				{
					Vulkan::Scope* currentSubPassScope = currentScope;
					HashSet<RHI::AttachmentID> initializedAttachmentIds{};

					while (currentSubPassScope != nullptr)
					{
						// Do image-layout/buffer transitions manually (if required)
						
						for (auto scopeAttachment : currentSubPassScope->attachments)
						{
							if (!scopeAttachment->IsImageAttachment() || scopeAttachment->GetFrameAttachment() == nullptr ||
								!scopeAttachment->GetFrameAttachment()->IsImageAttachment())
								continue;

							RHI::ImageScopeAttachment* imageScopeAttachment = (RHI::ImageScopeAttachment*)scopeAttachment;
							RHI::ImageFrameAttachment* imageFrameAttachment = (RHI::ImageFrameAttachment*)scopeAttachment->GetFrameAttachment();

							RHI::RHIResource* resource = imageFrameAttachment->GetResource(currentSubmissionIndex);
							if (resource == nullptr)
								continue;
							if (initializedAttachmentIds.Exists(imageFrameAttachment->GetId()))
								continue;
							if (resource->GetResourceType() != RHI::ResourceType::Buffer && resource->GetResourceType() != RHI::ResourceType::Texture &&
								resource->GetResourceType() != RHI::ResourceType::TextureView)
								continue;

							initializedAttachmentIds.Add(imageFrameAttachment->GetId());

							if (resource->GetResourceType() == RHI::ResourceType::Texture || resource->GetResourceType() == RHI::ResourceType::TextureView)
							{
								Vulkan::Texture* image = nullptr;//(Vulkan::Texture*)resource;
								if (resource->GetResourceType() == RHI::ResourceType::Texture)
								{
									image = (Vulkan::Texture*)resource;
								}
								else if (resource->GetResourceType() == RHI::ResourceType::TextureView)
								{
									TextureView* imageView = (TextureView*)resource;
									image = (Vulkan::Texture*)imageView->GetTexture();
								}

								VkImageLayout requiredLayout = image->curImageLayout;
								VkPipelineStageFlags dstStageMask = 0;
								VkAccessFlags dstAccessMask = 0;

								switch (scopeAttachment->GetUsage())
								{
								case RHI::ScopeAttachmentUsage::Color:
								case RHI::ScopeAttachmentUsage::Resolve:
									requiredLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
									dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
									dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
									break;
								case RHI::ScopeAttachmentUsage::DepthStencil:
									dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
									if (EnumHasFlag(scopeAttachment->GetAccess(), RHI::ScopeAttachmentAccess::Write))
									{
										requiredLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
										dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
									}
									else // Read only
									{
										requiredLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
										dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
									}
									break;
								case RHI::ScopeAttachmentUsage::SubpassInput:
								case RHI::ScopeAttachmentUsage::Shader:
									dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
									if (EnumHasFlag(scopeAttachment->GetAccess(), RHI::ScopeAttachmentAccess::Write))
									{
										requiredLayout = VK_IMAGE_LAYOUT_GENERAL;
										dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
									}
									else
									{
										requiredLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
										if (currentSubPassScope->IsComputePass())
											requiredLayout = VK_IMAGE_LAYOUT_GENERAL;
										dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
									}
									break;
								case RHI::ScopeAttachmentUsage::Copy:
									dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
									if (EnumHasFlag(scopeAttachment->GetAccess(), RHI::ScopeAttachmentAccess::Write))
									{
										requiredLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
										dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
									}
									else
									{
										requiredLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
										dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
									}
									break;
								default:
									continue;
								}

								// Usually only required for Output image, because it transitions to PRESENT_SRC_KHR at the end of rendering.
								// Also useful when queue family index of image doesn't match to what's required.
								if (image->curImageLayout != requiredLayout || (image->curFamilyIndex >= 0 && image->curFamilyIndex != currentScope->queue->GetFamilyIndex()))
								{
									VkImageMemoryBarrier imageBarrier{};
									imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
									if (image->curFamilyIndex < 0)
										image->curFamilyIndex = presentQueue->GetFamilyIndex();
									imageBarrier.srcQueueFamilyIndex = image->curFamilyIndex;
									imageBarrier.dstQueueFamilyIndex = currentScope->queue->GetFamilyIndex();
									imageBarrier.image = image->GetImage();
									imageBarrier.oldLayout = image->curImageLayout;
									imageBarrier.newLayout = requiredLayout;
									imageBarrier.srcAccessMask = 0;
									imageBarrier.dstAccessMask = dstAccessMask;

									imageBarrier.subresourceRange.aspectMask = image->aspectMask;
									imageBarrier.subresourceRange.baseMipLevel = 0;
									imageBarrier.subresourceRange.levelCount = image->mipLevels;
									imageBarrier.subresourceRange.baseArrayLayer = 0;
									imageBarrier.subresourceRange.layerCount = image->arrayLayers;

									vkCmdPipelineBarrier(cmdBuffer,
										VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
										dstStageMask,
										0,
										0, nullptr,
										0, nullptr,
										1, &imageBarrier);

									image->curImageLayout = requiredLayout;
									image->curFamilyIndex = currentScope->queue->GetFamilyIndex();
								}
							}
							else if (resource->GetResourceType() == RHI::ResourceType::Buffer)
							{
								Vulkan::Buffer* buffer = (Vulkan::Buffer*)resource;

								if (buffer->curFamilyIndex >= 0 && buffer->curFamilyIndex != currentScope->queue->GetFamilyIndex())
								{
									VkPipelineStageFlags dstStageMask = 0;
									VkAccessFlags dstAccessMask = 0;

									switch (scopeAttachment->GetUsage())
									{
									case RHI::ScopeAttachmentUsage::Shader:
										dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
										if (EnumHasFlag(scopeAttachment->GetAccess(), RHI::ScopeAttachmentAccess::Write))
										{
											dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
										}
										else
										{
											dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
										}
										break;
									case RHI::ScopeAttachmentUsage::Copy:
										dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
										if (EnumHasFlag(scopeAttachment->GetAccess(), RHI::ScopeAttachmentAccess::Write))
										{
											dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
										}
										else
										{
											dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
										}
										break;
									default:
										continue;
									}

									VkBufferMemoryBarrier bufferBarrier{};
									bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
									bufferBarrier.srcAccessMask = 0;
									bufferBarrier.dstAccessMask = dstAccessMask;
									bufferBarrier.srcQueueFamilyIndex = buffer->curFamilyIndex;
									bufferBarrier.dstQueueFamilyIndex = currentScope->queue->GetFamilyIndex();
									bufferBarrier.buffer = buffer->GetBuffer();
									bufferBarrier.offset = 0;
									bufferBarrier.size = buffer->GetBufferSize();

									vkCmdPipelineBarrier(cmdBuffer,
										VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
										dstStageMask,
										0,
										0, nullptr,
										1, &bufferBarrier,
										0, nullptr);

									buffer->curFamilyIndex = currentScope->queue->GetFamilyIndex();
								}
							}
						}

						currentSubPassScope = (Vulkan::Scope*)currentSubPassScope->nextSubPass;
					}
				}

				bool shouldNotExecuteAtAll = false;
				bool shouldNotExecuteButShouldClear = false;

				for (const auto& cond : currentScope->executeConditions)
				{
					if (!frameGraph->VariableExists(cond.variableName))
					{
						shouldNotExecuteAtAll = true;
						break;
					}
					const auto& value = frameGraph->GetVariable(currentSubmissionIndex, cond.variableName);
					bool result = cond.Compare(value);

					if (!result)
					{
						if (cond.shouldClear)
						{
							shouldNotExecuteButShouldClear = true;
							shouldNotExecuteAtAll = false;
						}
						else
						{
							shouldNotExecuteButShouldClear = false;
							shouldNotExecuteAtAll = true;
						}
						break;
					}
				}

				// Graphics operation
				if (!shouldNotExecuteAtAll && currentScope->queueClass == RHI::HardwareQueueClass::Graphics)
				{
					commandList->ClearShaderResourceGroups();

					VkRenderPassBeginInfo beginInfo{};
					beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					beginInfo.renderPass = renderPass->GetHandle();
					FrameBuffer* frameBuffer = currentScope->frameBuffers[currentSubmissionIndex];
					beginInfo.framebuffer = frameBuffer->GetHandle();
					beginInfo.clearValueCount = clearValues.GetSize();
					beginInfo.pClearValues = clearValues.GetData();

					beginInfo.renderArea.offset.x = 0;
					beginInfo.renderArea.offset.y = 0;
					beginInfo.renderArea.extent.width = frameBuffer->GetWidth();
					beginInfo.renderArea.extent.height = frameBuffer->GetHeight();

					vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
					{
						VkViewport viewport{};
						viewport.x = viewport.y = 0;
						viewport.width = frameBuffer->GetWidth();
						viewport.height = frameBuffer->GetHeight();
						viewport.minDepth = 0.0f;
						viewport.maxDepth = 1.0f;
						vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

						VkRect2D scissor{};
						scissor.offset.x = scissor.offset.y = 0;
						scissor.extent.width = viewport.width;
						scissor.extent.height = viewport.height;
						vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

						while (!shouldNotExecuteButShouldClear && currentScope != nullptr)
						{
							RHI::DrawList* drawList = currentScope->drawList;

							// Submit draw items
							for (int i = 0; drawList != nullptr && i < drawList->GetDrawItemCount(); i++)
							{
								for (auto srg : currentScope->externalShaderResourceGroups)
								{
									commandList->SetShaderResourceGroup(srg);
								}

								if (currentScope->passShaderResourceGroup)
									commandList->SetShaderResourceGroup(currentScope->passShaderResourceGroup);
								if (currentScope->subpassShaderResourceGroup)
									commandList->SetShaderResourceGroup(currentScope->subpassShaderResourceGroup);

								const auto& drawItemProperties = drawList->GetDrawItem(i);
								const RHI::DrawItem* drawItem = drawItemProperties.item;

								if (drawItem->enabled)
								{
									// TODO: Try using pipelineCollection instead of pipelineState

									// Bind Pipeline
									RHI::PipelineState* pipeline = drawItem->pipelineState;
									if (pipeline)
									{
										commandList->BindPipelineState(pipeline);
									}

									// Bind SRGs
									for (int j = 0; j < drawItem->shaderResourceGroupCount; j++)
									{
										if (drawItem->shaderResourceGroups[j] != nullptr)
										{
											commandList->SetShaderResourceGroup(drawItem->shaderResourceGroups[j]);
										}
									}

									for (int j = 0; j < drawItem->uniqueShaderResourceGroupCount; j++)
									{
										if (drawItem->uniqueShaderResourceGroups[j] != nullptr)
										{
											commandList->SetShaderResourceGroup(drawItem->uniqueShaderResourceGroups[j]);
										}
									}

									// Commit SRGs
									commandList->CommitShaderResources();

									// Draw Indexed
									commandList->BindVertexBuffers(0, drawItem->vertexBufferViewCount, drawItem->vertexBufferViews);

									if (drawItem->rootConstantSize > 0 && drawItem->rootConstants != nullptr && 
										(int)drawItem->rootConstantSize % 4 == 0)
									{
										commandList->SetRootConstants(0, (u32)drawItem->rootConstantSize / 4, drawItem->rootConstants);
									}
									
									if (drawItem->arguments.type == RHI::DrawArgumentsType::DrawArgumentsIndexed)
									{
										commandList->BindIndexBuffer(*drawItem->indexBufferView);
										commandList->DrawIndexed(drawItem->arguments.indexedArgs);
									}
									else if (drawItem->arguments.type == RHI::DrawArgumentsType::DrawArgumentsLinear)
									{
										commandList->DrawLinear(drawItem->arguments.linearArgs);
									}
								}
							}

							if (currentScope->nextSubPass == nullptr) // No more subpasses left
							{
								if (currentScope->IsSubPass())
								{
									const auto& renderPassDesc = currentScope->renderPass->desc;

									for (int i = 0; i < renderPassDesc.attachments.GetSize(); i++)
									{
										const auto& attachmentBinding = renderPassDesc.attachments[i];
										RHI::ScopeAttachment* scopeAttachment = currentScope->FindScopeAttachment(attachmentBinding.attachmentId);
										if (scopeAttachment == nullptr || scopeAttachment->GetFrameAttachment() == nullptr || 
											!scopeAttachment->GetFrameAttachment()->IsImageAttachment())
											continue;

										RHI::RHIResource* resource = scopeAttachment->GetFrameAttachment()->GetResource(currentSubmissionIndex);
										if (resource == nullptr)
											continue;

										if (resource->GetResourceType() == RHI::ResourceType::Texture)
										{
											Texture* texture = (Texture*)resource;
											texture->curImageLayout = attachmentBinding.finalLayout;
										}
										else if (resource->GetResourceType() == RHI::ResourceType::TextureView)
										{
											TextureView* textureView = (TextureView*)resource;
											Texture* texture = (Texture*)textureView->GetTexture();
											if (texture != nullptr)
											{
												texture->curImageLayout = attachmentBinding.finalLayout;
											}
										}
									}
								}

								commandList->currentSubpass = 0;

								for (const auto& [variableName, value] : currentScope->setVariablesAfterExecutionPerFrame)
								{
									scheduler->SetFrameGraphVariable(currentSubmissionIndex, variableName, value);
								}

								commandList->ClearShaderResourceGroups();
								break;
							}
							else // Go to next subpass
							{
								scopeIndex++;
								currentScope = (Vulkan::Scope*)currentScope->nextSubPass;
								vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
								commandList->currentSubpass++;
								commandList->ClearShaderResourceGroups();

								for (const auto& [variableName, value] : currentScope->setVariablesAfterExecutionPerFrame)
								{
									scheduler->SetFrameGraphVariable(currentSubmissionIndex, variableName, value);
								}

								for (const auto& [variableName, value] : currentScope->setVariablesAfterExecutionAllFrames)
								{
									for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
									{
										scheduler->SetFrameGraphVariable(i, variableName, value);
									}
								}
							}
						}
					}
					vkCmdEndRenderPass(cmdBuffer);
				}
				else if (currentScope->queueClass == RHI::HardwareQueueClass::Compute)
				{
					commandList->ClearShaderResourceGroups();

					// TODO: Add compute pass
					RHI::PipelineState* pipelineToUse = nullptr;

					for (RHI::PipelineState* pipeline : currentScope->usePipelines)
					{
						if (pipeline != nullptr && pipeline->GetPipelineType() == RHI::PipelineStateType::Compute)
						{
							pipelineToUse = pipeline;
						}
					}

					if (pipelineToUse != nullptr)
					{
						commandList->BindPipelineState(pipelineToUse);

						for (auto srg : currentScope->externalShaderResourceGroups)
						{
							commandList->SetShaderResourceGroup(srg);
						}

						if (currentScope->passShaderResourceGroup)
							commandList->SetShaderResourceGroup(currentScope->passShaderResourceGroup);
						if (currentScope->subpassShaderResourceGroup)
							commandList->SetShaderResourceGroup(currentScope->subpassShaderResourceGroup);

						commandList->CommitShaderResources();;

						commandList->Dispatch(Math::Max((u32)1, currentScope->groupCountX),
							Math::Max((u32)1, currentScope->groupCountY),
							Math::Max((u32)1, currentScope->groupCountZ));
					}					
				}
				else if (currentScope->queueClass == RHI::HardwareQueueClass::Transfer)
				{
					// TODO: Add transfer pass
				}

				// Execute compiled pipeline barriers (exit barriers)
				if (currentScope->barriers[currentSubmissionIndex].NotEmpty())
				{
					for (const auto& barrier : currentScope->barriers[currentSubmissionIndex])
					{
						vkCmdPipelineBarrier(cmdBuffer,
							barrier.srcStageMask, barrier.dstStageMask,
							0,
							barrier.memoryBarriers.GetSize(), barrier.memoryBarriers.GetData(),
							barrier.bufferBarriers.GetSize(), barrier.bufferBarriers.GetData(),
							barrier.imageBarriers.GetSize(), barrier.imageBarriers.GetData());

						for (const auto& transition : barrier.imageLayoutTransitions)
						{
							transition.image->curImageLayout = transition.layout;
							transition.image->curFamilyIndex = transition.queueFamilyIndex;
						}

						for (const auto& bufferTransition : barrier.bufferFamilyTransitions)
						{
							bufferTransition.buffer->curFamilyIndex = bufferTransition.queueFamilyIndex;
						}
					}
				}
			}

			commandList->currentPass = nullptr;
			commandList->currentSubpass = 0;
			
			// Transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR if required
			if (presentRequired)
			{
				for (int i = 0; i < scopeChain.Top()->presentSwapChains.GetSize(); i++)
				{
					Vulkan::Texture* image = (Vulkan::Texture*)scopeChain.Top()->presentSwapChains[i]->GetCurrentImage();
					if (image->curFamilyIndex < 0)
						image->curFamilyIndex = presentQueue->GetFamilyIndex();

					VkImageMemoryBarrier imageBarrier{};
					imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					if (image->curFamilyIndex < 0)
						image->curFamilyIndex = presentQueue->GetFamilyIndex();
					imageBarrier.srcQueueFamilyIndex = image->curFamilyIndex;
					imageBarrier.dstQueueFamilyIndex = presentQueue->GetFamilyIndex();
					imageBarrier.image = image->image;
					imageBarrier.oldLayout = image->curImageLayout;//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

					imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBarrier.subresourceRange.baseMipLevel = 0;
					imageBarrier.subresourceRange.levelCount = 1;
					imageBarrier.subresourceRange.baseArrayLayer = 0;
					imageBarrier.subresourceRange.layerCount = 1;

					vkCmdPipelineBarrier(cmdBuffer,
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &imageBarrier);

					image->curImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					image->curFamilyIndex = presentQueue->GetFamilyIndex();
				}
			}
		}
		commandList->End();
        
        // Manually reset (close) the fence. i.e. put it in unsignalled state
        result = vkResetFences(device->GetHandle(), 1, &scope->renderFinishedFences[currentSubmissionIndex]);

		static List<VkSemaphore> waitSemaphores{};
		static List<VkPipelineStageFlags> waitStages{};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		Vulkan::Scope* signallingScope = scopeChain.Top();
		
		if (!swapChainsUsedAsAttachmentForFirstTime.IsEmpty()) // Frame graph uses a SwapChain image for first time
		{
			submitInfo.waitSemaphoreCount = scope->waitSemaphores[currentSubmissionIndex].GetSize() + swapChainsUsedAsAttachmentForFirstTime.GetSize();
			if (waitSemaphores.GetSize() < submitInfo.waitSemaphoreCount)
				waitSemaphores.Resize(submitInfo.waitSemaphoreCount);
			if (waitStages.GetSize() < submitInfo.waitSemaphoreCount)
				waitStages.Resize(submitInfo.waitSemaphoreCount);
			
			// Wait semaphores from compiled FrameGraph, i.e. dependency on previous pass submissions.
			for (int i = 0; i < scope->waitSemaphores[currentSubmissionIndex].GetSize(); i++)
			{
				waitSemaphores[i] = scope->waitSemaphores[currentSubmissionIndex][i];
				waitStages[i] = scope->waitSemaphoreStageFlags[i];
			}

			int idx = 0;
			for (SwapChain* swapChain : swapChainsUsedAsAttachmentForFirstTime)
			{
				int swapChainIndex = frameGraph->presentSwapChains.IndexOf(swapChain);
				CE_ASSERT(swapChainIndex >= 0, "SwapChain not found in FrameGraph.");

				// We need to wait on image-acquire semaphore too
				waitSemaphores[scope->waitSemaphores[currentSubmissionIndex].GetSize() + idx] = compiler->imageAcquiredSemaphores[currentSubmissionIndex][swapChainIndex];
				waitStages[scope->waitSemaphores[currentSubmissionIndex].GetSize() + idx] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				idx++;
			}

			submitInfo.pWaitSemaphores = waitSemaphores.GetData();
			submitInfo.pWaitDstStageMask = waitStages.GetData();
		}
		else
		{
			submitInfo.waitSemaphoreCount = scope->waitSemaphores[currentSubmissionIndex].GetSize();
			if (submitInfo.waitSemaphoreCount > 0)
			{
				submitInfo.pWaitSemaphores = scope->waitSemaphores[currentSubmissionIndex].GetData();
				submitInfo.pWaitDstStageMask = scope->waitSemaphoreStageFlags.GetData();
			}
		}
		submitInfo.signalSemaphoreCount = signallingScope->signalSemaphores[currentSubmissionIndex].GetSize();
		submitInfo.pSignalSemaphores = signallingScope->signalSemaphores[currentSubmissionIndex].GetData();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandList->commandBuffer;
		
		//result = vkQueueSubmit(scope->queue->GetHandle(), 1, &submitInfo, scope->renderFinishedFences[currentSubmissionIndex]);
		bool success = scope->queue->Submit(1, &submitInfo, scope->renderFinishedFences[currentSubmissionIndex]);

		if (presentRequired && scopeChain.Top()->presentSwapChains.NotEmpty())
		{
			List<VkSwapchainKHR> swapchainKhrs{};
			List<u32> imageIndices{};

			for (int i = 0; i < scopeChain.Top()->presentSwapChains.GetSize(); i++)
			{
				VkSwapchainKHR swapchainKhr = ((Vulkan::SwapChain*)scopeChain.Top()->presentSwapChains[i])->GetHandle();
				if (swapchainKhr == nullptr)
					continue;

				swapchainKhrs.Add(swapchainKhr);
				imageIndices.Add(scopeChain.Top()->presentSwapChains[i]->GetCurrentImageIndex());
			}

			if (!swapchainKhrs.IsEmpty())
			{
				VkPresentInfoKHR presentInfo{};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				presentInfo.swapchainCount = swapchainKhrs.GetSize();
				presentInfo.pSwapchains = swapchainKhrs.GetData();
				presentInfo.pImageIndices = imageIndices.GetData();
				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = &signallingScope->signalSemaphores[currentSubmissionIndex][0];

				result = vkQueuePresentKHR(presentQueue->GetHandle(), &presentInfo);
			}
		}

		return result == VK_SUCCESS;
	}
    
} // namespace CE::Vulkan
