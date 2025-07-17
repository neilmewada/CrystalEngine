#include "EditorCore.h"

namespace CE::Editor
{

    TextureAssetThumbnailGen::TextureAssetThumbnailGen()
    {

    }

    TextureAssetThumbnailGen::~TextureAssetThumbnailGen()
    {
        
    }

    bool TextureAssetThumbnailGen::IsValidForAssetType(SubClass<Asset> assetClass)
    {
        return assetClass->IsSubclassOf<CE::Texture2D>();
    }

    bool TextureAssetThumbnailGen::StartProcessing()
    {
        AssetManager* assetManager = gEngine->GetAssetManager();

        blitShaderAsset = assetManager->LoadAssetAtPath<CE::Shader>("/Editor/Assets/Shaders/Utils/Blit");
        if (!blitShaderAsset)
            return false;

        RPI::ShaderCollection* blitShaderCollection = blitShaderAsset->GetShaderCollection();
        if (!blitShaderCollection)
            return false;

        RPI::Shader* blitShader = blitShaderCollection->At(0).shader;
        if (!blitShader)
            return false;

        for (const auto& assetPath : assetPaths)
        {
            Ref<CE::Texture> texture = assetManager->LoadAssetAtPath<CE::Texture>(assetPath);
            textureAssets.Add(texture);
        }

        Array<Job*> jobs{};

        jobCompletion.Reset(true);

		WeakRef<Self> self = this;

        jobCompletion.onFinish = [self]
            {
                if (auto selfLock = self.Lock())
                {
                    self->OnFinish();
                }
            };

        for (int i = 0; i < textureAssets.GetSize(); i++)
        {
            Ref<CE::Texture> texture = textureAssets[i];
            Name assetPath = assetPaths[i];
            if (texture == nullptr || texture->GetRpiTexture() == nullptr)
                continue;

			// We only support Texture2D for now
            if (!texture->IsOfType<Texture2D>())
				continue;

            u32 thumbnailResolution = this->thumbnailResolution;

            Job* job = new JobFunction([texture, blitShader, thumbnailResolution, assetPath](Job*)
                {
                    RPI::Texture* rpiTexture = texture->GetRpiTexture();
                    RHI::Texture* rhiTexture = rpiTexture->GetRhiTexture();

					////////////////////////////////////////////////////////
					// - Resources -

                    RHI::SamplerDescriptor samplerDesc{};
                    samplerDesc.borderColor = SamplerBorderColor::FloatTransparentBlack;
                    samplerDesc.addressModeU = samplerDesc.addressModeV = samplerDesc.addressModeW = SamplerAddressMode::ClampToBorder;
                    samplerDesc.enableAnisotropy = false;
                    samplerDesc.samplerFilterMode = FilterMode::Linear;

                    RHI::Sampler* sampler = RPISystem::Get().FindOrCreateSampler(samplerDesc);

                    RHI::TextureDescriptor outTextureDesc{};
                    outTextureDesc.width = thumbnailResolution;
                    outTextureDesc.height = thumbnailResolution;
                    outTextureDesc.depth = 1;
                    outTextureDesc.defaultHeapType = MemoryHeapType::Default;
                    outTextureDesc.arrayLayers = outTextureDesc.mipLevels = 1;
                    outTextureDesc.dimension = Dimension::Dim2D;
                    outTextureDesc.format = Format::R8G8B8A8_UNORM;
                    outTextureDesc.bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead;
                    outTextureDesc.sampleCount = 1;
                    outTextureDesc.name = "Output";

            		RHI::Texture* outTexture = RHI::gDynamicRHI->CreateTexture(outTextureDesc);

                    RHI::BufferDescriptor stagingBufferDesc{};
                    stagingBufferDesc.name = "Staging Buffer";
                    stagingBufferDesc.bindFlags = BufferBindFlags::StagingBuffer;
                    stagingBufferDesc.defaultHeapType = MemoryHeapType::ReadBack;
                    stagingBufferDesc.bufferSize = outTextureDesc.width * outTextureDesc.height * 4;
                    stagingBufferDesc.structureByteStride = stagingBufferDesc.bufferSize;

                    RHI::Buffer* stagingBuffer = RHI::gDynamicRHI->CreateBuffer(stagingBufferDesc);

                    RPI::Material* blitMaterial = new RPI::Material(blitShader);
                    blitMaterial->SetPropertyValue("_InputTexture", rpiTexture);
                    blitMaterial->SetPropertyValue("_InputSampler", sampler);
                    blitMaterial->FlushProperties();

					////////////////////////////////////////////////////////////
					// - Render Targets -

                    RHI::RenderTargetLayout rtLayout{};
                    RHI::RenderAttachmentLayout colorAttachment{};
                    colorAttachment.attachmentId = "Color";
                    colorAttachment.format = RHI::Format::R8G8B8A8_UNORM;
                    colorAttachment.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
                    colorAttachment.loadAction = RHI::AttachmentLoadAction::Clear;
                    colorAttachment.storeAction = RHI::AttachmentStoreAction::Store;
                    colorAttachment.loadActionStencil = RHI::AttachmentLoadAction::DontCare;
                    colorAttachment.storeActionStencil = RHI::AttachmentStoreAction::DontCare;
                    rtLayout.attachmentLayouts.Add(colorAttachment);

                    RHI::RenderTarget* renderTarget = RHI::gDynamicRHI->CreateRenderTarget(rtLayout);
                    RHI::RenderTargetBuffer* renderTargetBuffer = RHI::gDynamicRHI->CreateRenderTargetBuffer(renderTarget, { outTexture });

                    // Full screen quad
                    Array<RHI::VertexBufferView> fullscreenQuad = RPISystem::Get().GetFullScreenQuad();
                    RHI::DrawLinearArguments fullscreenQuadArgs = RPISystem::Get().GetFullScreenQuadDrawArgs();

                    /////////////////////////////////////////////////////
					// - Command List Submission -

                    RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
                    RHI::CommandList* cmdList = RHI::gDynamicRHI->AllocateCommandList(queue, CommandListType::Direct);
                    RHI::Fence* fence = RHI::gDynamicRHI->CreateFence();
                    
                    cmdList->Begin();
                    {
                        RHI::ResourceBarrierDescriptor barrier{};
                        barrier.subresourceRange = RHI::SubresourceRange::All();

                        barrier.fromState = ResourceState::Undefined;
                        barrier.toState = ResourceState::ColorOutput;
                        barrier.resource = outTexture;
                        cmdList->ResourceBarrier(1, &barrier);

                        AttachmentClearValue clearValue;
                        clearValue.clearValue = Vec4(0, 0, 0, 0);

                        cmdList->BeginRenderTarget(renderTarget, renderTargetBuffer, &clearValue);
                        {
                            RHI::ViewportState viewportState{};
                            viewportState.x = viewportState.y = 0;
                            viewportState.width = rhiTexture->GetWidth();
                            viewportState.height = rhiTexture->GetHeight();
                            viewportState.minDepth = 0;
                            viewportState.maxDepth = 1;
                            cmdList->SetViewports(1, &viewportState);

                            RHI::ScissorState scissorState{};
                            scissorState.x = scissorState.y = 0;
                            scissorState.width = viewportState.width;
                            scissorState.height = viewportState.height;
                            cmdList->SetScissors(1, &scissorState);

                            RHI::PipelineState* pipeline = blitMaterial->GetCurrentShader()->GetDefaultVariant()->GetPipeline();
                            cmdList->BindPipelineState(pipeline);

                            cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

                            cmdList->SetShaderResourceGroup(blitMaterial->GetShaderResourceGroup());

                            cmdList->CommitShaderResources();

                            cmdList->DrawLinear(fullscreenQuadArgs);
                        }
                        cmdList->EndRenderTarget();

                        barrier.fromState = ResourceState::ColorOutput;
                        barrier.toState = ResourceState::CopySource;
                        barrier.resource = outTexture;
                        cmdList->ResourceBarrier(1, &barrier);

                        barrier.fromState = ResourceState::Undefined;
                        barrier.toState = ResourceState::CopyDestination;
                        barrier.resource = stagingBuffer;
                        cmdList->ResourceBarrier(1, &barrier);

                        {
                            RHI::TextureToBufferCopy copy{};
                            copy.srcTexture = outTexture;
                            copy.baseArrayLayer = 0;
                            copy.layerCount = 1;
                            copy.mipSlice = 0;
                            copy.dstBuffer = stagingBuffer;
                            copy.bufferOffset = 0;

                            cmdList->CopyTextureRegion(copy);
                        }

                        barrier.fromState = ResourceState::CopyDestination;
                        barrier.toState = ResourceState::General;
                        barrier.resource = stagingBuffer;
                        cmdList->ResourceBarrier(1, &barrier);
                    }
                    cmdList->End();
                    
                    queue->Execute(1, &cmdList, fence);
                    fence->WaitForFence();

                    /////////////////////////////////////////////////////
					// - Save thumbnail to disk -

                    void* data;
                    stagingBuffer->Map(0, stagingBuffer->GetBufferSize(), &data);
                    {
                        CMImage image = CMImage::LoadRawImageFromMemory((unsigned char*)data, outTextureDesc.width, outTextureDesc.width,
                            CMImageFormat::RGBA8, CMImageSourceFormat::None, 
                            8, 8 * 4);

                        CMImageEncoder encoder{};

                        u64 size = encoder.GetCompressedSizeRequirement(image, CMImageSourceFormat::BC7);
                        if (size == 0)
                        {
                            SaveThumbnailToDisk(image, assetPath);
                        }
                        else
                        {
	                        u8* compressedData = new u8[size];
                            if (encoder.EncodeToBCn(image, compressedData, CMImageSourceFormat::BC7))
                            {
                                CMImage compressedImage = CMImage::LoadRawImageFromMemory(compressedData, outTextureDesc.width, outTextureDesc.width,
                                    CMImageFormat::BC7, CMImageSourceFormat::None, 8 / 4, 8);
                                SaveThumbnailToDisk(compressedImage, assetPath);
                            }
                            else
                            {
                                SaveThumbnailToDisk(image, assetPath);
                            }
							delete[] compressedData;
                        }
                    }
                    stagingBuffer->Unmap();

                    /////////////////////////////////////////////////////
					// - Cleanup resources -

                    delete blitMaterial;
                    RHI::gDynamicRHI->DestroyTexture(outTexture);
                    RHI::gDynamicRHI->DestroyBuffer(stagingBuffer);
                    RHI::gDynamicRHI->DestroyRenderTargetBuffer(renderTargetBuffer);
                    RHI::gDynamicRHI->DestroyRenderTarget(renderTarget);

                    RHI::gDynamicRHI->DestroyFence(fence);
                    RHI::gDynamicRHI->FreeCommandLists(1, &cmdList);

                }, true);

            jobs.Add(job);

            job->SetDependent(&jobCompletion);
        }

        started = true;

        for (Job* job : jobs)
        {
            job->Start();
        }

        jobCompletion.Start();

        return true;
    }

    bool TextureAssetThumbnailGen::IsProcessing()
    {
        return started && !jobCompletion.IsFinished();
    }

} // namespace CE

