#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    
	Scope::Scope(VulkanDevice* device, const RHI::ScopeDescriptor& desc) : Super(desc), device(device)
	{

	}

	Scope::~Scope()
	{
		vkDeviceWaitIdle(device->GetHandle());

		for (int i = 0; i < commandListsByFamilyIndexPerImage.GetSize(); i++)
		{
			for (int j = 0; j < commandListsByFamilyIndexPerImage[i].GetSize(); j++)
			{
				delete commandListsByFamilyIndexPerImage[i][j];
			}
			commandListsByFamilyIndexPerImage[i].Clear();
		}
		commandListsByFamilyIndexPerImage.Clear();

        if (renderPass)
        {
            // No need to destroy it. RenderPassCache manages it.
        }
        renderPass = nullptr;

		for (auto frameBuffer : frameBuffers)
		{
			delete frameBuffer;
		}
		frameBuffers.Clear();
        
		DestroySyncObjects();

		delete passShaderResourceGroup;
		passShaderResourceGroup = nullptr;
	}

	bool Scope::CompileInternal(const RHI::FrameGraphCompileRequest& compileRequest)
	{
		// TODO: Remove vkDeviceWaitIdle calls! They are bad for performance.
		vkDeviceWaitIdle(device->GetHandle());

		DestroySyncObjects();

		//delete passShaderResourceGroup;
		//passShaderResourceGroup = nullptr;
		//delete subpassShaderResourceGroup;
		//subpassShaderResourceGroup = nullptr;

		for (auto frameBuffer : frameBuffers)
		{
			delete frameBuffer;
		}
		frameBuffers.Clear();

		auto frameGraph = compileRequest.frameGraph;

		u32 imageCount = Math::Clamp<u32>(compileRequest.numFramesInFlight, 1, RHI::Limits::MaxSwapChainImageCount);

		if (frameGraph->GetSwapChainCount() > 0)
		{
			imageCount = frameGraph->GetSwapChain(0)->GetImageCount();
		}

		waitSemaphores.Clear();

		// Render Finished semaphores & fences
		for (int i = 0; i < imageCount; i++)
		{
			waitSemaphores.Add({});

			VkSemaphoreCreateInfo semaphoreCI{};
			semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkResult result = VK_SUCCESS;

			//VkSemaphore semaphore = nullptr;
			//result = vkCreateSemaphore(device->GetHandle(), &semaphoreCI, nullptr, &semaphore);
			if (result != VK_SUCCESS)
			{
				continue;
			}
			//renderFinishedSemaphores.Add(semaphore);

			VkFenceCreateInfo fenceCI{};
			fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			
			VkFence fence = nullptr;
			result = vkCreateFence(device->GetHandle(), &fenceCI, VULKAN_CPU_ALLOCATOR, &fence);
			if (result != VK_SUCCESS)
			{
				continue;
			}

			renderFinishedFences.Add(fence);
			signalSemaphores.Add({});
			signalSemaphoresByConsumerScope.Add({});

			// 1 signal semaphore for 1 consumer
			for (auto consumerRhiScope : consumers)
			{
				Vulkan::Scope* consumerScope = (Vulkan::Scope*)consumerRhiScope;
				while (consumerScope->prevSubPass != nullptr)
				{
					consumerScope = (Vulkan::Scope*)consumerScope->prevSubPass;
				}

				if (signalSemaphoresByConsumerScope[i].KeyExists(consumerScope->id))
					continue;

				VkSemaphore signalSemaphore = nullptr;
				vkCreateSemaphore(device->GetHandle(), &semaphoreCI, VULKAN_CPU_ALLOCATOR, &signalSemaphore);
				signalSemaphores[i].Add(signalSemaphore);
				signalSemaphoresByConsumerScope[i].Add(consumerScope->id, signalSemaphore);
			}
			// No consumers exist, then just signal 1 semaphore
			if (consumers.IsEmpty())
			{
				VkSemaphore signalSemaphore = nullptr;
				vkCreateSemaphore(device->GetHandle(), &semaphoreCI, VULKAN_CPU_ALLOCATOR, &signalSemaphore);
				signalSemaphores[i].Add(signalSemaphore);
			}
		}

		if (operation == RHI::ScopeOperation::Compute)
		{
			String::IsAlphabet('a');
		}

		if (IsGraphicsPass())
		{
			if (prevSubPass == nullptr && nextSubPass != nullptr)
			{
				// TODO: Fix RenderPass with multiple subpasses
				Vulkan::Scope* next = this;
				this->subpassIndex = 0;
				int i = 0;

				while (next != nullptr)
				{
					// Assign appropriate subpass indices
					next->subpassIndex = i++;
					next = (Vulkan::Scope*)next->nextSubPass;
				}

				RenderPassCache* rpCache = device->GetRenderPassCache();
				RenderPass::Descriptor descriptor{};
				RenderPass::BuildDescriptor(this, descriptor);
				renderPass = rpCache->FindOrCreate(descriptor);

				next = this;

				while (next != nullptr)
				{
					bool foundPipelineLayout = false;
					bool foundSubpassPipelineLayout = false;

					for (RHI::PipelineState* rhiPipelineState : next->usePipelines)
					{
						auto pipelineState = (Vulkan::PipelineState*)rhiPipelineState;
						Pipeline* pipeline = pipelineState->GetPipeline();
						if (!pipeline || pipeline->GetPipelineType() != RHI::PipelineStateType::Graphics)
							continue;
						GraphicsPipeline* graphicsPipeline = (GraphicsPipeline*)pipeline;
						graphicsPipeline->Compile(renderPass, subpassIndex);

						// Setup SRG
						auto pipelineLayout = (Vulkan::PipelineLayout*)rhiPipelineState->GetPipelineLayout();
						if (pipelineLayout != nullptr && !foundPipelineLayout && pipelineLayout->srgLayouts.KeyExists(RHI::SRGType::PerPass))
						{
							foundPipelineLayout = true;
							const RHI::ShaderResourceGroupLayout& srgLayout = pipelineLayout->srgLayouts[RHI::SRGType::PerPass];
							next->passShaderResourceGroup = RHI::gDynamicRHI->CreateShaderResourceGroup(srgLayout);

							// Bind Pass attachments to SRG
						}

						if (pipelineLayout != nullptr && !foundSubpassPipelineLayout && pipelineLayout->srgLayouts.KeyExists(RHI::SRGType::PerSubPass))
						{
							foundSubpassPipelineLayout = true;
							const RHI::ShaderResourceGroupLayout& srgLayout = pipelineLayout->srgLayouts[RHI::SRGType::PerSubPass];
							next->subpassShaderResourceGroup = RHI::gDynamicRHI->CreateShaderResourceGroup(srgLayout);

							// Bind Pass attachments to SRG
						}
					}

					next->renderPass = renderPass;
					next = (Vulkan::Scope*)next->nextSubPass;
				}
			}
			else if (!IsSubPass())
			{
				// Compile Render Pass
	            RenderPassCache* rpCache = device->GetRenderPassCache();
	            RenderPass::Descriptor descriptor{};
	            RenderPass::BuildDescriptor(this, descriptor);
	            renderPass = rpCache->FindOrCreate(descriptor);
				subpassIndex = 0;
				bool foundPipelineLayout = false;

				if (!passSrgLayout.IsEmpty())
				{
					passShaderResourceGroup = RHI::gDynamicRHI->CreateShaderResourceGroup(passSrgLayout);

					// Bind Pass attachments to SRG

					for (auto scopeAttachment : attachments)
					{
						if (scopeAttachment->GetFrameAttachment() == nullptr)
							continue;
						if (scopeAttachment->GetUsage() != RHI::ScopeAttachmentUsage::Shader)
							continue;

						// Find the actual name in shader
						Name attachmentName = scopeAttachment->GetShaderInputName();
						RHI::FrameAttachment* frameAttachment = scopeAttachment->GetFrameAttachment();

						for (int imageIdx = 0; imageIdx < RHI::Limits::MaxSwapChainImageCount; imageIdx++)
						{
							if (frameAttachment->IsImageAttachment())
							{
								RHI::RHIResource* resource = frameAttachment->GetResource(imageIdx);
								if (resource == nullptr || resource->GetResourceType() != RHI::ResourceType::Texture)
									break;

								if (resource->GetResourceType() == ResourceType::Texture)
								{
									RHI::Texture* image = (RHI::Texture*)resource;

									passShaderResourceGroup->Bind(imageIdx, attachmentName, image);
								}
								else if (resource->GetResourceType() == ResourceType::TextureView)
								{
									RHI::TextureView* image = (RHI::TextureView*)resource;

									passShaderResourceGroup->Bind(imageIdx, attachmentName, image);
								}
								else
								{
									break;
								}
							}
							else if (frameAttachment->IsBufferAttachment())
							{
								RHI::RHIResource* resource = frameAttachment->GetResource(imageIdx);
								if (resource == nullptr || resource->GetResourceType() != RHI::ResourceType::Buffer)
									break;

								RHI::Buffer* buffer = (RHI::Buffer*)resource;

								passShaderResourceGroup->Bind(imageIdx, attachmentName, buffer);
							}
						}
					}

					passShaderResourceGroup->FlushBindings();
				}

				// Pre-Compile Shader Pipelines
				for (RHI::PipelineState* rhiPipelineState : usePipelines)
				{
					auto pipelineState = (Vulkan::PipelineState*)rhiPipelineState;
					Pipeline* pipeline = pipelineState->GetPipeline();
					if (!pipeline || pipeline->GetPipelineType() != RHI::PipelineStateType::Graphics)
						continue;

					GraphicsPipeline* graphicsPipeline = (GraphicsPipeline*)pipeline;
					graphicsPipeline->Compile(renderPass, subpassIndex);

					// Setup SRG
					auto pipelineLayout = (Vulkan::PipelineLayout*)rhiPipelineState->GetPipelineLayout();
					if (pipelineLayout != nullptr && !foundPipelineLayout && pipelineLayout->srgLayouts.KeyExists(RHI::SRGType::PerPass))
					{
						foundPipelineLayout = true;
						const RHI::ShaderResourceGroupLayout& srgLayout = pipelineLayout->srgLayouts[RHI::SRGType::PerPass];
						passShaderResourceGroup = RHI::gDynamicRHI->CreateShaderResourceGroup(srgLayout);

						// Bind Pass attachments to SRG

						for (auto scopeAttachment : attachments)
						{
							Name attachmentName = scopeAttachment->GetId();
							if (scopeAttachment->GetFrameAttachment() == nullptr)
								continue;
							if (scopeAttachment->GetUsage() != RHI::ScopeAttachmentUsage::Shader)
								continue;

							RHI::FrameAttachment* frameAttachment = scopeAttachment->GetFrameAttachment();

							for (int imageIdx = 0; imageIdx < RHI::Limits::MaxSwapChainImageCount; imageIdx++)
							{
								if (frameAttachment->IsImageAttachment())
								{
									RHI::RHIResource* resource = frameAttachment->GetResource(imageIdx);
									if (resource == nullptr)
										break;

									if (resource->GetResourceType() == RHI::ResourceType::Texture)
									{
										RHI::Texture* image = (RHI::Texture*)resource;

										passShaderResourceGroup->Bind(imageIdx, scopeAttachment->GetShaderInputName(), image);
									}
									else if (resource->GetResourceType() == ResourceType::TextureView)
									{
										RHI::TextureView* image = (RHI::TextureView*)resource;

										passShaderResourceGroup->Bind(imageIdx, scopeAttachment->GetShaderInputName(), image);
									}
									else
									{
										break;
									}
								}
								else if (frameAttachment->IsBufferAttachment())
								{
									RHI::RHIResource* resource = frameAttachment->GetResource(imageIdx);
									if (resource == nullptr || resource->GetResourceType() != RHI::ResourceType::Buffer)
										break;

									RHI::Buffer* buffer = (RHI::Buffer*)resource;

									passShaderResourceGroup->Bind(imageIdx, scopeAttachment->GetShaderInputName(), buffer);
								}
							}
						}

						passShaderResourceGroup->FlushBindings();
					}
				}
			}
			else
			{
				return true;
			}

			for (int i = 0; i < imageCount; i++)
			{
				frameBuffers.Add(new FrameBuffer(device, this, i));
			}
		}
		else if (IsComputePass())
		{
			renderPass = nullptr;
			subpassIndex = 0;
			bool foundPipelineLayout = false;

			for (RHI::PipelineState* rhiPipelineState : usePipelines)
			{
				auto pipelineState = (Vulkan::PipelineState*)rhiPipelineState;
				Pipeline* pipeline = pipelineState->GetPipeline();
				if (!pipeline || pipeline->GetPipelineType() != RHI::PipelineStateType::Compute)
					continue;

				// Setup SRG
				auto pipelineLayout = (Vulkan::PipelineLayout*)rhiPipelineState->GetPipelineLayout();
				if (pipelineLayout != nullptr && !foundPipelineLayout && pipelineLayout->srgLayouts.KeyExists(RHI::SRGType::PerPass))
				{
					foundPipelineLayout = true;
					const RHI::ShaderResourceGroupLayout& srgLayout = pipelineLayout->srgLayouts[RHI::SRGType::PerPass];
					if (passShaderResourceGroup == nullptr)
					{
						passShaderResourceGroup = RHI::gDynamicRHI->CreateShaderResourceGroup(srgLayout);
					}

					// Bind Pass attachments to SRG

					for (auto scopeAttachment : attachments)
					{
						Name attachmentName = scopeAttachment->GetId();
						if (scopeAttachment->GetFrameAttachment() == nullptr)
							continue;
						if (scopeAttachment->GetUsage() != RHI::ScopeAttachmentUsage::Shader)
							continue;

						RHI::FrameAttachment* frameAttachment = scopeAttachment->GetFrameAttachment();

						for (int imageIdx = 0; imageIdx < RHI::Limits::MaxSwapChainImageCount; imageIdx++)
						{
							if (frameAttachment->IsImageAttachment())
							{
								RHI::RHIResource* resource = frameAttachment->GetResource(imageIdx);
								if (resource == nullptr)
									break;

								if (resource->GetResourceType() == RHI::ResourceType::Texture)
								{
									RHI::Texture* image = (RHI::Texture*)resource;

									passShaderResourceGroup->Bind(imageIdx, scopeAttachment->GetShaderInputName(), image);
								}
								else if (resource->GetResourceType() == ResourceType::TextureView)
								{
									RHI::TextureView* image = (RHI::TextureView*)resource;

									passShaderResourceGroup->Bind(imageIdx, scopeAttachment->GetShaderInputName(), image);
								}
								else
								{
									break;
								}
							}
							else if (frameAttachment->IsBufferAttachment())
							{
								RHI::RHIResource* resource = frameAttachment->GetResource(imageIdx);
								if (resource == nullptr || resource->GetResourceType() != RHI::ResourceType::Buffer)
									break;

								RHI::Buffer* buffer = (RHI::Buffer*)resource;

								passShaderResourceGroup->Bind(imageIdx, scopeAttachment->GetShaderInputName(), buffer);
							}
						}
					}

					passShaderResourceGroup->FlushBindings();
				}
			}
		}

		return true;
	}

	void Scope::DestroySyncObjects()
	{
		vkDeviceWaitIdle(device->GetHandle());

		for (int i = 0; i < signalSemaphores.GetSize(); i++)
		{
			for (auto semaphore : signalSemaphores[i])
			{
				vkDestroySemaphore(device->GetHandle(), semaphore, VULKAN_CPU_ALLOCATOR);
			}
			signalSemaphores[i].Clear();
			signalSemaphoresByConsumerScope[i].Clear();
		}
		signalSemaphores.Clear();
		signalSemaphoresByConsumerScope.Clear();

		//for (VkSemaphore semaphore : renderFinishedSemaphores)
		//{
		//	vkDestroySemaphore(device->GetHandle(), semaphore, nullptr);
		//}
		//renderFinishedSemaphores.Clear();

		for (VkFence fence : renderFinishedFences)
		{
			vkDestroyFence(device->GetHandle(), fence, VULKAN_CPU_ALLOCATOR);
		}
		renderFinishedFences.Clear();
	}

} // namespace CE::Vulkan
