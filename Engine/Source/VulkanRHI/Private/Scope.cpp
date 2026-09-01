#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    
	Scope::Scope(Device* device, const RHI::ScopeDescriptor& desc) : Super(desc), device(device)
	{

	}

	Scope::~Scope()
	{
        if (renderPass)
        {
            // No need to destroy it. RenderPassCache manages it.
        }
        renderPass = nullptr;
        
		DestroySyncObjects();

		delete passShaderResourceGroup;
		passShaderResourceGroup = nullptr;
	}

	bool Scope::CompileInternal(const RHI::FrameGraphCompileRequest& compileRequest)
	{
		auto frameGraph = compileRequest.frameGraph;

		u32 imageCount = Math::Clamp<u32>(compileRequest.numFramesInFlight, 1, RHI::Limits::MaxSwapChainImageCount);
		Vulkan::SwapChain* presentSwapChain = nullptr;

		if (frameGraph->GetSwapChainCount() > 0)
		{
			imageCount = ((Vulkan::SwapChain*)frameGraph->GetSwapChain(0))->GetImageCount();
		}

		if (this->presentSwapChains.GetSize() > 0)
		{
			presentSwapChain = (Vulkan::SwapChain*)this->presentSwapChains[0];
		}

		imageCount = Math::Min(imageCount, RHI::Limits::MaxSwapChainImageCount);

		if (IsGraphicsPass())
		{
			if (!IsSubPass())
			{
				// Compile Render Pass
	            RenderPassCache* rpCache = device->GetRenderPassCache();
	            VulkanRenderPass::Descriptor descriptor{};
	            VulkanRenderPass::BuildDescriptor(this, descriptor);
	            renderPass = rpCache->FindOrCreate(descriptor);
				subpassIndex = 0;
				bool foundPipelineLayout = false;

				if (!passSrgLayout.IsEmpty())
				{
                    RHI::ShaderResourceGroupDescriptor passSrgDesc{};
                    passSrgDesc.layout = passSrgLayout;
                    
					passShaderResourceGroup = RHI::gDynamicRHI->CreateShaderResourceGroup(passSrgDesc);

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
                        
                        RHI::ShaderResourceGroupDescriptor srgDesc{};
                        srgDesc.layout = srgLayout;
                        
						passShaderResourceGroup = RHI::gDynamicRHI->CreateShaderResourceGroup(srgDesc);

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
                        RHI::ShaderResourceGroupDescriptor srgDesc{};
                        srgDesc.layout = srgLayout;
                        
						passShaderResourceGroup = RHI::gDynamicRHI->CreateShaderResourceGroup(srgDesc);
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
		
	}

} // namespace CE::Vulkan
