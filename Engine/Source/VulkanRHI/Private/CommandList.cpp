#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	CommandList::CommandList(VulkanDevice* device, VkCommandBuffer commandBuffer, RHI::CommandListType type, u32 queueFamilyIndex, VkCommandPool pool)
		: device(device)
		, commandBuffer(commandBuffer)
		, level(type == RHI::CommandListType::Direct ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY)
		, queueFamilyIndex(queueFamilyIndex)
		, pool(pool)
	{
		curQueueFlags = device->queueFamilyProperties[queueFamilyIndex].queueFlags;
		this->commandListType = type;
		srgManager = device->GetShaderResourceManager();
	}

	CommandList::~CommandList()
	{
		for (int setNumber = 0; setNumber < RHI::Limits::Pipeline::MaxShaderResourceGroupCount; setNumber++)
		{
			if (commitedSRGsBySetNumber[setNumber] != nullptr)
			{
				commitedSRGsBySetNumber[setNumber]->usageCount--;
			}
			commitedSRGsBySetNumber[setNumber] = nullptr;
		}

		vkFreeCommandBuffers(device->GetHandle(), pool, 1, &commandBuffer);
	}

	void CommandList::SetShaderResourceGroups(const ArrayView<RHI::ShaderResourceGroup*>& srgs)
	{
		for (auto rhiSrg : srgs)
		{
			auto srg = (Vulkan::ShaderResourceGroup*)rhiSrg;
			boundSRGs[(int)srg->GetSRGType()] = srg;
		}
	}

	void CommandList::ClearShaderResourceGroups()
	{
		for (int i = 0; i < boundSRGs.GetSize(); i++)
		{
			boundSRGs[i] = nullptr;
		}
	}

	void CommandList::SetRootConstants(u32 offset, u32 num32BitValues, const void* srcData)
	{
		if (boundPipeline == nullptr || !boundPipeline->HasPushConstants())
			return;

		VkPushConstantRange pushConstantRange = boundPipeline->GetPushConstant();

		vkCmdPushConstants(commandBuffer, boundPipeline->GetVkPipelineLayout(), pushConstantRange.stageFlags, offset, num32BitValues * sizeof(u32), srcData);
	}

	void CommandList::SetViewports(u32 count, ViewportState* viewports)
	{
		Array<VkViewport> vkViewports{};
		if (vkViewports.GetSize() < count)
			vkViewports.Resize(count);

		for (int i = 0; i < count; i++)
		{
			vkViewports[i].x = viewports[i].x;
			vkViewports[i].y = viewports[i].y;
			vkViewports[i].width = viewports[i].width;
			vkViewports[i].height = viewports[i].height;
			vkViewports[i].minDepth = 0.0f;
			vkViewports[i].maxDepth = 1.0f;
		}
		vkCmdSetViewport(commandBuffer, 0, count, vkViewports.GetData());
	}

	void CommandList::SetScissors(u32 count, ScissorState* scissors)
	{
		Array<VkRect2D> vkScissors{};
		if (vkScissors.GetSize() < count)
			vkScissors.Resize(count);

		for (int i = 0; i < count; i++)
		{
			vkScissors[i].offset.x = scissors[i].x;
			vkScissors[i].offset.y = scissors[i].y;
			vkScissors[i].extent.width = scissors[i].width;
			vkScissors[i].extent.height = scissors[i].height;
		}

		vkCmdSetScissor(commandBuffer, 0, count, vkScissors.GetData());
	}

	void CommandList::CommitShaderResources()
	{
		if (boundPipeline == nullptr)
			return;

		StaticArray<
			FixedArray<Vulkan::ShaderResourceGroup*, RHI::Limits::Pipeline::MaxShaderResourceGroupCount>,
			RHI::Limits::Pipeline::MaxShaderResourceGroupCount
		> srgsToMerge{};

		for (auto srg : boundSRGs)
		{
			if (!srg)
				continue;

			int setNumber = srg->GetSetNumber();
			srg->FlushBindings();

			srgsToMerge[setNumber].Add(srg);
		}

		for (int setNumber = 0; setNumber < RHI::Limits::Pipeline::MaxShaderResourceGroupCount; setNumber++)
		{
			if (srgsToMerge[setNumber].GetSize() == 0)
				continue;

			if (srgsToMerge[setNumber].GetSize() == 1)
			{
				if (needsSrgCommit || commitedSRGsBySetNumber[setNumber] != srgsToMerge[setNumber][0]->GetDescriptorSet()) // SRG has changed
				{
					DescriptorSet* descriptorSet = srgsToMerge[setNumber][0]->GetDescriptorSet();

					srgsToMerge[setNumber][0]->currentImageIndex = currentImageIndex;
					srgsToMerge[setNumber][0]->FlushBindings();

					if (commitedSRGsBySetNumber[setNumber] != nullptr)
					{
						--commitedSRGsBySetNumber[setNumber]->usageCount;
					}
					commitedSRGsBySetNumber[setNumber] = srgsToMerge[setNumber][0]->GetDescriptorSet();
					++commitedSRGsBySetNumber[setNumber]->usageCount;

					VkDescriptorSet descriptorSetHandle = commitedSRGsBySetNumber[setNumber]->GetHandle();
					vkCmdBindDescriptorSets(commandBuffer, boundPipeline->GetBindPoint(),
						boundPipeline->GetVkPipelineLayout(), setNumber, 1, &descriptorSetHandle, 0, nullptr);
				}
			}
			else // > 1 (Merge SRGs)
			{
				auto first = &*srgsToMerge[setNumber].begin();
				auto last = &*(srgsToMerge[setNumber].end() - 1);
				auto mergedSrg = srgManager->FindOrCreateMergedSRG(srgsToMerge[setNumber].GetSize(), srgsToMerge[setNumber].GetData());

				//if (commitedSRGsBySetNumber[setNumber] != (Vulkan::ShaderResourceGroup*)mergedSrg) // SRG has changed
				{
					mergedSrg->currentImageIndex = currentImageIndex;
					mergedSrg->FlushBindings();

					if (commitedSRGsBySetNumber[setNumber] != nullptr)
					{
						--commitedSRGsBySetNumber[setNumber]->usageCount;
					}
					commitedSRGsBySetNumber[setNumber] = mergedSrg->GetDescriptorSet();
					++commitedSRGsBySetNumber[setNumber]->usageCount;

					VkDescriptorSet descriptorSet = commitedSRGsBySetNumber[setNumber]->GetHandle();
					vkCmdBindDescriptorSets(commandBuffer, boundPipeline->GetBindPoint(),
						boundPipeline->GetVkPipelineLayout(), setNumber, 1, &descriptorSet, 0, nullptr);
				}
			}
		}

		needsSrgCommit = false;
	}

	void CommandList::BindPipelineState(RHI::PipelineState* rhiPipelineState)
	{
		if (rhiPipelineState == nullptr)
			return;

		RHI::PipelineStateType pipelineType = rhiPipelineState->GetPipelineType();
		Vulkan::PipelineState* pipelineState = (Vulkan::PipelineState*)rhiPipelineState;

		// No need to bind the same pipeline again
		if (boundPipeline == pipelineState->GetPipeline())
			return;
		
		boundPipeline = pipelineState->GetPipeline();
		
		if (pipelineType == RHI::PipelineStateType::Graphics)
		{
			Vulkan::GraphicsPipeline* gfxPipeline = (Vulkan::GraphicsPipeline*)boundPipeline;
			VkPipeline vkPipeline = gfxPipeline->FindOrCompile(currentPass, currentSubpass);

			if (curPipelineType != VK_PIPELINE_BIND_POINT_GRAPHICS)
			{
				needsSrgCommit = true;
			}

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
			curPipelineType = VK_PIPELINE_BIND_POINT_GRAPHICS;
		}
		else if (pipelineType == RHI::PipelineStateType::Compute)
		{
			if (curPipelineType != VK_PIPELINE_BIND_POINT_COMPUTE)
			{
				needsSrgCommit = true;
			}

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, boundPipeline->GetPipeline());
			curPipelineType = VK_PIPELINE_BIND_POINT_COMPUTE;
		}
	}

	void CommandList::BindVertexBuffers(u32 firstInputSlot, u32 count, const RHI::VertexBufferView* bufferViews)
	{
		count = Math::Min(count, RHI::Limits::Pipeline::MaxVertexInputSlotCount);

		FixedArray<VkBuffer, RHI::Limits::Pipeline::MaxVertexInputSlotCount> buffers{};
		FixedArray<VkDeviceSize, RHI::Limits::Pipeline::MaxVertexInputSlotCount> offsets{};

		for (int i = 0; i < count; i++)
		{
			Vulkan::Buffer* buffer = (Vulkan::Buffer*)bufferViews[i].GetBuffer();
			buffers.Add(buffer->GetBuffer());
			offsets.Add((VkDeviceSize)bufferViews[i].GetByteOffset());
		}

		vkCmdBindVertexBuffers(commandBuffer, firstInputSlot, count, buffers.GetData(), offsets.GetData());
	}

	void CommandList::BindIndexBuffer(const RHI::IndexBufferView& bufferView)
	{
		Vulkan::Buffer* buffer = (Vulkan::Buffer*)bufferView.GetBuffer();
		VkIndexType indexType = {};

		switch (bufferView.GetIndexFormat())
		{
		case RHI::IndexFormat::Uint16:
			indexType = VK_INDEX_TYPE_UINT16;
			break;
		case RHI::IndexFormat::Uint32:
			indexType = VK_INDEX_TYPE_UINT32;
			break;
		}

		vkCmdBindIndexBuffer(commandBuffer, buffer->GetBuffer(), bufferView.GetByteOffset(), indexType);
	}

	void CommandList::DrawIndexed(const DrawIndexedArguments& args)
	{
		vkCmdDrawIndexed(commandBuffer, args.indexCount, args.instanceCount, args.firstIndex, args.vertexOffset, args.firstInstance);
	}

	void CommandList::DrawLinear(const DrawLinearArguments& args)
	{
		vkCmdDraw(commandBuffer, args.vertexCount, args.instanceCount, args.vertexOffset, args.firstInstance);
	}

	void CommandList::Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
	{
		vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void CommandList::ResourceBarrier(u32 count, RHI::ResourceBarrierDescriptor* barriers)
	{
		if (count == 0)
			return;

		for (int i = 0; i < count; i++)
		{
			VkPipelineStageFlags srcStageMask = 0;
			VkPipelineStageFlags dstStageMask = 0;

			List<VkBufferMemoryBarrier> bufferBarriers{};
			List<VkImageMemoryBarrier> imageBarriers{};

			const RHI::ResourceBarrierDescriptor& barrierInfo = barriers[i];
			if (barrierInfo.resource == nullptr)
				continue;
			
			RHI::DeviceObjectType resourceType = barrierInfo.resource->GetDeviceObjectType();
			if (resourceType == RHI::DeviceObjectType::Texture)
			{
				Vulkan::Texture* texture = (Vulkan::Texture*)barrierInfo.resource;
				if (texture->GetImage() == nullptr)
					continue;

				VkImageMemoryBarrier imageBarrier{};
				imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageBarrier.image = texture->GetImage();

				imageBarrier.subresourceRange.aspectMask = texture->aspectMask;
				if (barrierInfo.subresourceRange.IsAll())
				{
					imageBarrier.subresourceRange.baseArrayLayer = 0;
					imageBarrier.subresourceRange.layerCount = texture->arrayLayers;
					imageBarrier.subresourceRange.baseMipLevel = 0;
					imageBarrier.subresourceRange.levelCount = texture->mipLevels;
				}
				else
				{
					imageBarrier.subresourceRange.baseArrayLayer = barrierInfo.subresourceRange.baseArrayLayer;
					imageBarrier.subresourceRange.layerCount = barrierInfo.subresourceRange.layerCount;
					imageBarrier.subresourceRange.baseMipLevel = barrierInfo.subresourceRange.baseMipLevel;
					imageBarrier.subresourceRange.levelCount = barrierInfo.subresourceRange.levelCount;
				}
				
				// Change queue family ownership of the image
				if (texture->curFamilyIndex >= 0 && texture->curFamilyIndex != queueFamilyIndex)
				{
					imageBarrier.srcQueueFamilyIndex = texture->curFamilyIndex;
					imageBarrier.dstQueueFamilyIndex = queueFamilyIndex;
				}
				else
				{
					imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				}

				switch (barrierInfo.fromState) // OLD state
				{
				case RHI::ResourceState::Undefined:
					srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					imageBarrier.srcAccessMask = 0;
					break;
				case RHI::ResourceState::General:
					srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | 
						VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
						VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
					break;
				case RHI::ResourceState::BlitSource:
				case RHI::ResourceState::CopySource:
					srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case RHI::ResourceState::BlitDestination:
				case RHI::ResourceState::CopyDestination:
					srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case RHI::ResourceState::DepthRead:
					srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					break;
				case RHI::ResourceState::DepthWrite:
					srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case RHI::ResourceState::FragmentShaderResource:
					srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::NonFragmentShaderResource:
					srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | 
						VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::ColorOutput:
					srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case RHI::ResourceState::ShaderWrite:
					srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
					break;
				case RHI::ResourceState::Present:
					srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					imageBarrier.srcAccessMask = 0;
					break;
				default:
					continue;
				}

				switch (barrierInfo.toState) // NEW state
				{
				case RHI::ResourceState::Undefined: // Cannot transition into Undefined state.
					continue;
				case RHI::ResourceState::General:
					dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
						VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
						VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
					break;
				case RHI::ResourceState::BlitSource:
				case RHI::ResourceState::CopySource:
					dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case RHI::ResourceState::BlitDestination:
				case RHI::ResourceState::CopyDestination:
					dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case RHI::ResourceState::DepthRead:
					dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					break;
				case RHI::ResourceState::DepthWrite:
					dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case RHI::ResourceState::FragmentShaderResource:
					dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::NonFragmentShaderResource:
					dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
						VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::ColorOutput:
					dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case RHI::ResourceState::ShaderWrite:
					dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
					break;
				case RHI::ResourceState::Present:
					dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					imageBarrier.dstAccessMask = 0;
					break;
				default:
					continue;
				}

				vkCmdPipelineBarrier(commandBuffer,
					srcStageMask, dstStageMask,
					0,
					0, nullptr,
					0, nullptr,
					1, &imageBarrier);

				texture->curImageLayout = imageBarrier.newLayout;
				texture->curFamilyIndex = queueFamilyIndex;
			}
			else if (resourceType == DeviceObjectType::TextureView)
			{
				Vulkan::TextureView* textureView = (Vulkan::TextureView*)barrierInfo.resource;
				if (textureView->GetTexture() == nullptr || textureView->GetImageView() == nullptr)
					continue;
				Vulkan::Texture* texture = (Vulkan::Texture*)textureView->GetTexture();
				if (texture->GetImage() == nullptr)
					continue;

				VkImageMemoryBarrier imageBarrier{};
				imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageBarrier.image = texture->GetImage();

				imageBarrier.subresourceRange.aspectMask = textureView->aspectMask;
				if (barrierInfo.subresourceRange.IsAll())
				{
					imageBarrier.subresourceRange.baseArrayLayer = textureView->baseArrayLayer;
					imageBarrier.subresourceRange.layerCount = textureView->arrayLayerCount;
					imageBarrier.subresourceRange.baseMipLevel = textureView->baseMipLevel;
					imageBarrier.subresourceRange.levelCount = textureView->mipLevelCount;
				}
				else
				{
					imageBarrier.subresourceRange.baseArrayLayer = barrierInfo.subresourceRange.baseArrayLayer;
					imageBarrier.subresourceRange.layerCount = barrierInfo.subresourceRange.layerCount;
					imageBarrier.subresourceRange.baseMipLevel = barrierInfo.subresourceRange.baseMipLevel;
					imageBarrier.subresourceRange.levelCount = barrierInfo.subresourceRange.levelCount;
				}

				// Change queue family ownership of the image
				if (texture->curFamilyIndex >= 0 && texture->curFamilyIndex != queueFamilyIndex)
				{
					imageBarrier.srcQueueFamilyIndex = texture->curFamilyIndex;
					imageBarrier.dstQueueFamilyIndex = queueFamilyIndex;
				}
				else
				{
					imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				}

				switch (barrierInfo.fromState) // OLD state
				{
				case RHI::ResourceState::Undefined:
					srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					imageBarrier.srcAccessMask = 0;
					break;
				case RHI::ResourceState::General:
					srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
						VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
						VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
					break;
				case RHI::ResourceState::BlitSource:
				case RHI::ResourceState::CopySource:
					srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case RHI::ResourceState::BlitDestination:
				case RHI::ResourceState::CopyDestination:
					srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case RHI::ResourceState::DepthRead:
					srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					break;
				case RHI::ResourceState::DepthWrite:
					srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case RHI::ResourceState::FragmentShaderResource:
					srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::NonFragmentShaderResource:
					srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
						VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::ColorOutput:
					srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case RHI::ResourceState::ShaderWrite:
					srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
					break;
				case RHI::ResourceState::Present:
					srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					imageBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					imageBarrier.srcAccessMask = 0;
					break;
				default:
					continue;
				}

				switch (barrierInfo.toState) // NEW state
				{
				case RHI::ResourceState::Undefined: // Cannot transition into Undefined state.
					continue;
				case RHI::ResourceState::General:
					dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
						VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
						VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
					break;
				case RHI::ResourceState::BlitSource:
				case RHI::ResourceState::CopySource:
					dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case RHI::ResourceState::BlitDestination:
				case RHI::ResourceState::CopyDestination:
					dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case RHI::ResourceState::DepthRead:
					dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					break;
				case RHI::ResourceState::DepthWrite:
					dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case RHI::ResourceState::FragmentShaderResource:
					dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::NonFragmentShaderResource:
					dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
						VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::ColorOutput:
					dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case RHI::ResourceState::ShaderWrite:
					dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
					break;
				case RHI::ResourceState::Present:
					dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					imageBarrier.dstAccessMask = 0;
					break;
				default:
					continue;
				}

				vkCmdPipelineBarrier(commandBuffer,
					srcStageMask, dstStageMask,
					0,
					0, nullptr,
					0, nullptr,
					1, &imageBarrier);

				texture->curImageLayout = imageBarrier.newLayout;
				texture->curFamilyIndex = queueFamilyIndex;
			}
			else if (resourceType == RHI::DeviceObjectType::Buffer)
			{
				Vulkan::Buffer* buffer = (Vulkan::Buffer*)barrierInfo.resource;
				if (buffer->GetBuffer() == nullptr)
					continue;

				VkBufferMemoryBarrier bufferBarrier{};
				bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

				bufferBarrier.buffer = buffer->GetBuffer();
				bufferBarrier.offset = 0;
				bufferBarrier.size = buffer->GetBufferSize();

				if (buffer->curFamilyIndex >= 0 && buffer->curFamilyIndex != queueFamilyIndex)
				{
					bufferBarrier.srcQueueFamilyIndex = buffer->curFamilyIndex;
					bufferBarrier.dstQueueFamilyIndex = queueFamilyIndex;
				}
				else
				{
					bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				}

				switch (barrierInfo.fromState) // OLD state
				{
				case RHI::ResourceState::Undefined:
					srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					bufferBarrier.srcAccessMask = 0;
					break;
				case RHI::ResourceState::General: // A "general" buffer
					srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
					break;
				case RHI::ResourceState::ConstantBuffer:
					srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::VertexBuffer:
					srcStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
					break;
				case RHI::ResourceState::IndexBuffer:
					srcStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_INDEX_READ_BIT;
					break;
				case RHI::ResourceState::CopySource:
					srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case RHI::ResourceState::CopyDestination:
					srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case RHI::ResourceState::FragmentShaderResource:
					srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::NonFragmentShaderResource:
					srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
						VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::ShaderWrite:
					srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
					break;
                default:
                    continue;
				}

				switch (barrierInfo.toState) // NEW state
				{
				case RHI::ResourceState::General: // A "general" buffer
				case RHI::ResourceState::ConstantBuffer:
					dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::VertexBuffer:
					dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
					bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
					break;
				case RHI::ResourceState::IndexBuffer:
					dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
					bufferBarrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
					break;
				case RHI::ResourceState::CopySource:
					dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					bufferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case RHI::ResourceState::CopyDestination:
					dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
					bufferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case RHI::ResourceState::FragmentShaderResource:
					dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::NonFragmentShaderResource:
					dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
						VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				case RHI::ResourceState::ShaderWrite:
					dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
					break;
                default:
                    continue;
				}

				vkCmdPipelineBarrier(commandBuffer,
					srcStageMask, dstStageMask,
					0,
					0, nullptr,
					1, &bufferBarrier,
					0, nullptr);

				buffer->curFamilyIndex = queueFamilyIndex;
			}

		}

	}

	void CommandList::CopyTextureRegion(const BufferToTextureCopy& region)
	{
		if (region.srcBuffer == nullptr || region.dstTexture == nullptr)
			return;

		Vulkan::Texture* dstTexture = (Vulkan::Texture*)region.dstTexture;
		Vulkan::Buffer* srcBuffer = (Vulkan::Buffer*)region.srcBuffer;

		VkBufferImageCopy copy{};
		copy.imageOffset = { 0, 0, 0 };
		copy.imageExtent.width = dstTexture->GetWidth(region.mipSlice);
		copy.imageExtent.height = dstTexture->GetHeight(region.mipSlice);
		copy.imageExtent.depth = dstTexture->GetDepth(region.mipSlice);

		copy.bufferOffset = region.bufferOffset;
		copy.bufferImageHeight = 0; // 0 means data is tightly packed
		copy.bufferRowLength = 0; // 0 means data is tightly packed

		copy.imageSubresource.aspectMask = dstTexture->aspectMask;
		copy.imageSubresource.baseArrayLayer = region.baseArrayLayer;
		copy.imageSubresource.layerCount = region.layerCount;
		copy.imageSubresource.mipLevel = region.mipSlice;

		vkCmdCopyBufferToImage(commandBuffer,
			srcBuffer->GetBuffer(),
			dstTexture->GetImage(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copy);
	}

	void CommandList::CopyTextureRegion(const TextureToBufferCopy& region)
	{
		if (region.srcTexture == nullptr || region.dstBuffer == nullptr)
			return;

		Vulkan::Texture* srcTexture = (Vulkan::Texture*)region.srcTexture;
		Vulkan::Buffer* dstBuffer = (Vulkan::Buffer*)region.dstBuffer;

		VkBufferImageCopy copy{};
		copy.imageOffset = { 0, 0, 0 };
		copy.imageExtent.width = srcTexture->GetWidth(region.mipSlice);
		copy.imageExtent.height = srcTexture->GetHeight(region.mipSlice);
		copy.imageExtent.depth = srcTexture->GetDepth(region.mipSlice);
		
		copy.bufferOffset = region.bufferOffset;
		copy.bufferImageHeight = 0; // 0 means data is tightly packed
		copy.bufferRowLength = 0; // 0 means data is tightly packed

		copy.imageSubresource.aspectMask = srcTexture->aspectMask;
		copy.imageSubresource.baseArrayLayer = region.baseArrayLayer;
		copy.imageSubresource.layerCount = region.layerCount;
		copy.imageSubresource.mipLevel = region.mipSlice;

		vkCmdCopyImageToBuffer(commandBuffer,
			srcTexture->GetImage(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstBuffer->GetBuffer(),
			1, &copy);
	}

	void CommandList::CopyBufferRegion(const BufferCopy& copy)
	{
		if (copy.srcBuffer == nullptr || copy.dstBuffer == nullptr)
			return;

		Vulkan::Buffer* srcBuffer = (Vulkan::Buffer*)copy.srcBuffer;
		Vulkan::Buffer* dstBuffer = (Vulkan::Buffer*)copy.dstBuffer;

		VkBufferCopy region{};
		region.size = copy.totalByteSize;
		region.srcOffset = copy.srcOffset;
		region.dstOffset = copy.dstOffset;

		vkCmdCopyBuffer(commandBuffer, srcBuffer->GetBuffer(), dstBuffer->GetBuffer(), 1, &region);
	}

	void CommandList::Begin()
	{
		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo);

		boundPipeline = nullptr;
		
		for (int i = 0; i < boundSRGs.GetSize(); i++)
		{
			boundSRGs[i] = nullptr;
		}

		for (int i = 0; i < commitedSRGsBySetNumber.GetSize(); i++)
		{
			if (commitedSRGsBySetNumber[i] != nullptr)
			{
				commitedSRGsBySetNumber[i]->usageCount--;
			}
			commitedSRGsBySetNumber[i] = nullptr;
		}
	}

	void CommandList::End()
	{
		vkEndCommandBuffer(commandBuffer);
	}

	void CommandList::BeginRenderTarget(RHI::RenderTarget* rhiRenderTarget, RHI::RenderTargetBuffer* renderTargetBuffer, RHI::AttachmentClearValue* clearValuesPerAttachment)
	{
		if (rhiRenderTarget == nullptr || renderTargetBuffer == nullptr)
			return;

		Vulkan::RenderTarget* renderTarget = (Vulkan::RenderTarget*)rhiRenderTarget;
		Vulkan::FrameBuffer* framebuffer = (Vulkan::FrameBuffer*)renderTargetBuffer;
		currentPass = renderTarget->renderPass;
		currentSubpass = 0;

		u32 attachmentCount = renderTarget->GetAttachmentCount();

		FixedArray<VkClearValue, RHI::Limits::Pipeline::MaxRenderAttachmentCount> clearValues{};
		for (int i = 0; i < attachmentCount; i++)
		{
			auto attachmentUsage = renderTarget->renderPass->GetAttachmentUsage(i);
			
			VkClearValue clearValue;
			memset(&clearValue, 0, sizeof(clearValue));

			switch (attachmentUsage)
			{
			case ScopeAttachmentUsage::Color:
			case ScopeAttachmentUsage::Resolve:
				memcpy(clearValue.color.float32, clearValuesPerAttachment[i].clearValue.xyzw, sizeof(f32[4]));
				break;
			case ScopeAttachmentUsage::DepthStencil:
				clearValue.depthStencil.depth = clearValuesPerAttachment[i].clearValueDepth;
				clearValue.depthStencil.stencil = clearValuesPerAttachment[i].clearValueStencil;
				break;
			}

			clearValues.Add(clearValue);
		}

		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.clearValueCount = attachmentCount;
		beginInfo.pClearValues = clearValues.GetData();
		
		beginInfo.renderPass = renderTarget->renderPass->GetHandle();
		beginInfo.framebuffer = framebuffer->GetHandle();

		beginInfo.renderArea.offset = { 0, 0 };
		beginInfo.renderArea.extent.width = framebuffer->GetWidth();
		beginInfo.renderArea.extent.height = framebuffer->GetHeight();
		
		vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandList::EndRenderTarget()
	{
		currentPass = nullptr;
		currentSubpass = 0;

		vkCmdEndRenderPass(commandBuffer);
	}

} // namespace CE::Vulkan
