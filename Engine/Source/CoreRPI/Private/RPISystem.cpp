#include "CoreRPI.h"

namespace CE::RPI
{
    static constexpr f32 quadVertexData[] = {
        // Positions
        -1.0f, -1.0f, 0, 0,
        1.0f, -1.0f, 0, 0,
        -1.0f, 1.0f, 0, 0,
        -1.0f, 1.0f, 0, 0,
        1.0f, -1.0f, 0, 0,
        1.0f, 1.0f, 0, 0,
        // UVs
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    static constexpr f32 textQuadVertexData[] = {
        // Positions
        0.0f, 0.0f, 0, 0,
        1.0f, 0.0f, 0, 0,
        0.0f, 1.0f, 0, 0,
        0.0f, 1.0f, 0, 0,
        1.0f, 0.0f, 0, 0,
        1.0f, 1.0f, 0, 0,
        // UVs
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    RPISystem& RPISystem::Get()
    {
		static RPISystem instance{};
        return instance;
    }

    void RPISystem::Initialize()
    {
        if (isInitialized)
            return;

        isFirstTick = true;
        isInitialized = true;

        CreateDefaultTextures();
        CreateFullScreenQuad();

        PassSystem::Get().Initialize();

        builtinDrawTags.Clear();

        for (int i = 1; i < (int)BuiltinDrawItemTag::COUNT; ++i)
        {
            RHI::DrawListTag tag{};
            RHI::DrawListTagRegistry* registry = GetDrawListTagRegistry();

            switch ((BuiltinDrawItemTag)i)
            {
            case BuiltinDrawItemTag::Depth:
                tag = registry->AcquireTag("depth");
	            break;
            case BuiltinDrawItemTag::Opaque:
                tag = registry->AcquireTag("opaque");
	            break;
            case BuiltinDrawItemTag::Shadow:
                tag = registry->AcquireTag("shadow");
	            break;
            case BuiltinDrawItemTag::UI:
                tag = registry->AcquireTag("ui");
	            break;
            case BuiltinDrawItemTag::Transparent:
                tag = registry->AcquireTag("transparent");
                break;
            case BuiltinDrawItemTag::Skybox:
                tag = registry->AcquireTag("skybox");
                break;
            case BuiltinDrawItemTag::None:
            case BuiltinDrawItemTag::COUNT:
                continue;
            }
            builtinDrawTags[(BuiltinDrawItemTag)i] = tag;
        }
    }

    void RPISystem::PostInitialize(const RPISystemInitInfo& initInfo)
    {
        this->standardShader = initInfo.standardShader;
        this->iblConvolutionShader = initInfo.iblConvolutionShader;
        this->textureGenShader = initInfo.textureGenShader;

        if (standardShader != nullptr)
	    {
		    for (const ShaderCollection::Item& shaderItem : *standardShader)
		    {
		    	if (!shaderItem.shader)
		    		continue;

		    	RPI::ShaderVariant* variant = shaderItem.shader->GetVariant(shaderItem.shader->GetDefaultVariantIndex());
		    	if (!variant)
		    		continue;

		    	viewSrgLayout = variant->GetSrgLayout(RHI::SRGType::PerView);
		    	sceneSrgLayout = variant->GetSrgLayout(RHI::SRGType::PerScene);

                for (auto& variable : sceneSrgLayout.variables)
                {
                    variable.shaderStages |= RHI::ShaderStage::Compute;
                }

                for (auto& variable : viewSrgLayout.variables)
                {
					variable.shaderStages |= RHI::ShaderStage::Compute;
                }
		    }
	    }

        if (iblConvolutionShader != nullptr)
        {
            CreateBrdfLutTexture();
        }
    }

    void RPISystem::CreateBrdfLutTexture()
    {
        if (!iblConvolutionShader)
            return;

        // - Create BRDF Lut Texture by rendering a full screen quad

        RPI::ShaderVariant* shaderVariant = nullptr;
        const Name lookupName = "BRDF Generator";

        for (const auto& item : *iblConvolutionShader)
        {
            RPI::Shader* rpiShader = item.shader;
            if (!rpiShader)
                continue;
            if (rpiShader->GetName() != lookupName)
                continue;

            shaderVariant = rpiShader->GetVariant(rpiShader->GetDefaultVariantIndex());
        }

        if (!shaderVariant)
            return;

        RPI::TextureDescriptor rpiDesc{};

        RHI::TextureDescriptor& brdfLutDesc = rpiDesc.texture;
        brdfLutDesc.name = "BRDF LUT Texture";
        brdfLutDesc.sampleCount = 1;
        brdfLutDesc.arrayLayers = 1;
        brdfLutDesc.mipLevels = 1;
        brdfLutDesc.defaultHeapType = RHI::MemoryHeapType::Default;
        brdfLutDesc.width = brdfLutDesc.height = 512;
        brdfLutDesc.depth = 1;
        brdfLutDesc.bindFlags = RHI::TextureBindFlags::Color | RHI::TextureBindFlags::ShaderRead;
        brdfLutDesc.format = RHI::Format::R8G8_UNORM;
        brdfLutDesc.dimension = RHI::Dimension::Dim2D;

        brdfLutTexture = new RPI::Texture(rpiDesc);
        RHI::Texture* brdfLut = brdfLutTexture->GetRhiTexture();

        RHI::RenderTarget* brdfLutRT = nullptr;
        RHI::RenderTargetBuffer* brdfLutRTB = nullptr;
        {
            RHI::RenderTargetLayout rtLayout{};
            RHI::RenderAttachmentLayout colorAttachment{};
            colorAttachment.attachmentId = "BRDF LUT";
            colorAttachment.format = RHI::Format::R8G8_UNORM;
            colorAttachment.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
            colorAttachment.loadAction = RHI::AttachmentLoadAction::Clear;
            colorAttachment.storeAction = RHI::AttachmentStoreAction::Store;
            colorAttachment.multisampleState.sampleCount = 1;
            rtLayout.attachmentLayouts.Add(colorAttachment);

            brdfLutRT = RHI::gDynamicRHI->CreateRenderTarget(rtLayout);
            brdfLutRTB = RHI::gDynamicRHI->CreateRenderTargetBuffer(brdfLutRT, { brdfLut });
        }

        defer(&)
        {
            delete brdfLutRT;
            delete brdfLutRTB;
        };

        auto queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
        auto cmdList = RHI::gDynamicRHI->AllocateCommandList(queue);
        auto fence = RHI::gDynamicRHI->CreateFence(false);

        const auto& fullscreenQuad = GetFullScreenQuad();
        const auto& fullscreenQuadArgs = GetFullScreenQuadDrawArgs();

        RHI::AttachmentClearValue clearValue{ .clearValue = Vec4(0, 0, 0, 0) };

        cmdList->Begin();
        {
            RHI::ResourceBarrierDescriptor barrier{};
            barrier.resource = brdfLut;
            barrier.fromState = RHI::ResourceState::Undefined;
            barrier.toState = RHI::ResourceState::ColorOutput;
            barrier.subresourceRange = RHI::SubresourceRange::All();
            cmdList->ResourceBarrier(1, &barrier);

            cmdList->ClearShaderResourceGroups();

            cmdList->BeginRenderTarget(brdfLutRT, brdfLutRTB, &clearValue);
            {
                RHI::ViewportState viewportState{};
                viewportState.x = viewportState.y = 0;
                viewportState.width = brdfLut->GetWidth();
                viewportState.height = brdfLut->GetHeight();
                viewportState.minDepth = 0;
                viewportState.maxDepth = 1;
                cmdList->SetViewports(1, &viewportState);

                RHI::ScissorState scissorState{};
                scissorState.x = scissorState.y = 0;
                scissorState.width = viewportState.width;
                scissorState.height = viewportState.height;
                cmdList->SetScissors(1, &scissorState);

                cmdList->BindPipelineState(shaderVariant->GetPipeline());

                cmdList->BindVertexBuffers(0, fullscreenQuad.GetSize(), fullscreenQuad.GetData());

                cmdList->DrawLinear(fullscreenQuadArgs);
            }
            cmdList->EndRenderTarget();

            barrier.resource = brdfLut;
            barrier.fromState = RHI::ResourceState::ColorOutput;
            barrier.toState = RHI::ResourceState::FragmentShaderResource;
            cmdList->ResourceBarrier(1, &barrier);
        }
        cmdList->End();

        queue->Execute(1, &cmdList, fence);
        fence->WaitForFence();

        RHI::gDynamicRHI->DestroyFence(fence); fence = nullptr;
        RHI::gDynamicRHI->FreeCommandLists(1, &cmdList);
    }

    void RPISystem::Shutdown()
    {
        if (!isInitialized)
            return;

        standardShader = nullptr;
        isInitialized = false;

        for (const auto& [builtinTag, drawListTag] : builtinDrawTags)
        {
            if (!drawListTag.IsValid())
                continue;
            GetDrawListTagRegistry()->ReleaseTag(drawListTag);
        }
        builtinDrawTags.Clear();

	    for (int i = (int)scenes.GetSize() - 1; i >= 0; --i)
	    {
			delete scenes[i];
	    }
        scenes.Clear();

        PassSystem::Get().Shutdown();

        for (auto buffer : vertexBuffers)
        {
            RHI::gDynamicRHI->DestroyBuffer(buffer);
        }
        vertexBuffers.Clear();

        builtinTextures.Clear();

        delete defaultNormalTex; defaultNormalTex = nullptr;
        delete defaultAlbedoTex; defaultAlbedoTex = nullptr;
        delete defaultRoughnessTex; defaultRoughnessTex = nullptr;
        delete defaultDepthTex; defaultDepthTex = nullptr;

        delete brdfLutTexture; brdfLutTexture = nullptr;

        {
            LockGuard lock{ samplerCacheMutex };

            for (auto [_, sampler] : samplerCache)
            {
                RHI::gDynamicRHI->DestroySampler(sampler);
            }
            samplerCache.Clear();
        }

        {
            LockGuard lock{ rhiDestructionQueueMutex };

            for (int i = rhiDestructionQueue.GetSize() - 1; i >= 0; i--)
            {
                auto& entry = rhiDestructionQueue[i];
                delete entry.resource; entry.resource = nullptr;
            }

            rhiDestructionQueue.Clear();
        }
    }

    void RPISystem::SimulationTick(u32 imageIndex)
    {
        ZoneScoped;

        {
            LockGuard lock{ rhiDestructionQueueMutex };

            for (int i = rhiDestructionQueue.GetSize() - 1; i >= 0; i--)
            {
                auto& entry = rhiDestructionQueue[i];
	            if (entry.frameCounter >= RHI::Limits::MaxSwapChainImageCount)
	            {
                    delete entry.resource; entry.resource = nullptr;
                
                    rhiDestructionQueue.RemoveAt(i);
                    continue;
	            }

                entry.frameCounter++;
            }
        }

        if (isFirstTick)
        {
            startTime = clock();
            isFirstTick = false;
        }

        currentTime = GetCurrentTime();

        for (Scene* scene : scenes)
        {
            scene->Simulate(currentTime, imageIndex);
        }
    }

    void RPISystem::RenderTick(u32 imageIndex)
    {
        ZoneScoped;

        MaterialSystem::Get().Update(imageIndex);

        for (Scene* scene : scenes)
        {
            scene->PrepareRender(currentTime, imageIndex);
        }

        for (Scene* scene : scenes)
        {
            scene->OnRenderEnd();
        }
    }

    f32 RPISystem::GetCurrentTime() const
    {
        return (f32)(clock() - startTime) / CLOCKS_PER_SEC;
    }

    void RPISystem::CreateFullScreenQuad()
    {
        RHI::BufferDescriptor quadVertexBufferDesc{};
        quadVertexBufferDesc.name = "Quad Vertex Buffer";
        quadVertexBufferDesc.bindFlags = RHI::BufferBindFlags::VertexBuffer;
        quadVertexBufferDesc.defaultHeapType = RHI::MemoryHeapType::Default;
        quadVertexBufferDesc.bufferSize = sizeof(quadVertexData);

        RHI::Buffer* quadVertexBuffer = RHI::gDynamicRHI->CreateBuffer(quadVertexBufferDesc);
        quadVertexBuffer->UploadData(quadVertexData, quadVertexBuffer->GetBufferSize(), 0);
        vertexBuffers.Add(quadVertexBuffer);

        quadVertexBufferViews.Add(RHI::VertexBufferView(quadVertexBuffer, 0, sizeof(Vec4) * 6, sizeof(Vec4)));
        quadVertexBufferViews.Add(RHI::VertexBufferView(quadVertexBuffer, sizeof(Vec4) * 6, sizeof(Vec2) * 6, sizeof(Vec2)));

        quadDrawArgs.firstInstance = 0;
        quadDrawArgs.instanceCount = 1;
        quadDrawArgs.vertexOffset = 0;
        quadDrawArgs.vertexCount = 6;

        RHI::Buffer* textQuadVertexBuffer = RHI::gDynamicRHI->CreateBuffer(quadVertexBufferDesc);
        textQuadVertexBuffer->UploadData(textQuadVertexData, textQuadVertexBuffer->GetBufferSize(), 0);
        vertexBuffers.Add(textQuadVertexBuffer);

        textQuadVertexBufferViews.Add(RHI::VertexBufferView(textQuadVertexBuffer, 0, sizeof(Vec4) * 6, sizeof(Vec4)));
        textQuadVertexBufferViews.Add(RHI::VertexBufferView(textQuadVertexBuffer, sizeof(Vec4) * 6, sizeof(Vec2) * 6, sizeof(Vec2)));

        textQuadDrawArgs.firstInstance = 0;
        textQuadDrawArgs.instanceCount = 1;
        textQuadDrawArgs.vertexOffset = 0;
        textQuadDrawArgs.vertexCount = 6;
    }

    RHI::Sampler* RPISystem::FindOrCreateSampler(const RHI::SamplerDescriptor& desc)
    {
        LockGuard<SharedMutex> lock{ samplerCacheMutex };

        SIZE_T hash = desc.GetHash();
        RHI::Sampler* sampler = samplerCache[hash];
        if (sampler != nullptr)
            return sampler;

        sampler = RHI::gDynamicRHI->CreateSampler(desc);
        samplerCache[hash] = sampler;
        return sampler;
    }

    void RPISystem::QueueDestroy(RHI::RHIResource* rhiResource)
    {
        if (!isInitialized)
        {
            delete rhiResource;
	        return;
        }

        LockGuard lock{ rhiDestructionQueueMutex };

        rhiDestructionQueue.Add({ .resource = rhiResource, .frameCounter = 0 });
    }

    RPI::Texture* RPISystem::FindBuiltinTexture(const Name& name)
    {
        if (!builtinTextures.KeyExists(name))
            return nullptr;
        return builtinTextures[name];
    }

    void RPISystem::CreateDefaultTextures()
    {
        auto prevTime = clock();

        RHI::SamplerDescriptor samplerDesc{};
        samplerDesc.addressModeU = samplerDesc.addressModeV = samplerDesc.addressModeW =
	        RHI::SamplerAddressMode::Repeat;
        samplerDesc.samplerFilterMode = RHI::FilterMode::Linear;
        samplerDesc.enableAnisotropy = false;

        RHI::TextureDescriptor textureDesc{};
        textureDesc.name = "Default Albedo";
        textureDesc.arrayLayers = 1;
        textureDesc.mipLevels = 1;
        textureDesc.dimension = RHI::Dimension::Dim2D;
        textureDesc.width = 4;
        textureDesc.height = 4;
        textureDesc.depth = 1;
        textureDesc.defaultHeapType = RHI::MemoryHeapType::Default;
        textureDesc.sampleCount = 1;
        textureDesc.bindFlags = RHI::TextureBindFlags::ShaderRead;
        textureDesc.format = RHI::Format::R8G8B8A8_UNORM;
        auto albedoTex = RHI::gDynamicRHI->CreateTexture(textureDesc);

        defaultAlbedoTex = new RPI::Texture(albedoTex, samplerDesc);

        builtinTextures["white"] = defaultAlbedoTex;

        textureDesc.name = "Default Normal";
        textureDesc.format = RHI::Format::R8G8B8A8_UNORM;
        auto normalTex = RHI::gDynamicRHI->CreateTexture(textureDesc);

        defaultNormalTex = new RPI::Texture(normalTex, samplerDesc);

        builtinTextures["bump"] = defaultNormalTex;

        textureDesc.name = "Default Roughness";
        textureDesc.format = RHI::Format::R8_UNORM;
        auto roughnessTex = RHI::gDynamicRHI->CreateTexture(textureDesc);

        defaultRoughnessTex = new RPI::Texture(roughnessTex, samplerDesc);

        textureDesc.name = "Default Depth";
        textureDesc.format = RHI::gDynamicRHI->GetAvailableDepthOnlyFormats().GetFirst();
        auto depthTex = RHI::gDynamicRHI->CreateTexture(textureDesc);

        defaultDepthTex = new RPI::Texture(depthTex, samplerDesc);

        const int numPixels = textureDesc.width * textureDesc.height;
        const u64 albedoSize = textureDesc.width * textureDesc.height * 4;
        const u64 normalSize = textureDesc.width * textureDesc.height * 4;
        const u64 roughnessSize = textureDesc.width * textureDesc.height;

        const u64 normalOffset = albedoSize;
        const u64 roughnessOffset = albedoSize + normalSize;

        RHI::BufferDescriptor stagingDesc{};
        stagingDesc.name = "Texture Copy Buffer";
        stagingDesc.bufferSize = albedoSize + roughnessSize + normalSize;
        stagingDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
        stagingDesc.defaultHeapType = RHI::MemoryHeapType::Upload;

        RHI::Buffer* stagingBuffer = RHI::gDynamicRHI->CreateBuffer(stagingDesc);

        void* ptr = nullptr;
        stagingBuffer->Map(0, stagingBuffer->GetBufferSize(), &ptr);
        {
            u8* albedoData = (u8*)ptr;
            u8* normalData = (u8*)ptr + normalOffset;
            u8* roughnessData = (u8*)ptr + roughnessOffset;

            constexpr u8 u8Max = NumericLimits<u8>::Max();

            for (int i = 0; i < numPixels; i++)
            {
                *(albedoData + i * 4) = u8Max;
                *(albedoData + i * 4 + 1) = u8Max;
                *(albedoData + i * 4 + 2) = u8Max;
                *(albedoData + i * 4 + 3) = u8Max;

                *(normalData + i * 4) = (u8)127; // R
                *(normalData + i * 4 + 1) = (u8)127; // G
                *(normalData + i * 4 + 2) = (u8)255; // B
                *(normalData + i * 4 + 3) = 0; // A

                *(roughnessData + i) = u8Max;
            }
        }
        stagingBuffer->Unmap();

        RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
        RHI::CommandList* cmdList = RHI::gDynamicRHI->AllocateCommandList(queue);
        RHI::Fence* fence = RHI::gDynamicRHI->CreateFence(false);

        cmdList->Begin();
        {
            RHI::ResourceBarrierDescriptor barrier{};
            barrier.resource = stagingBuffer;
            barrier.fromState = RHI::ResourceState::Undefined;
            barrier.toState = RHI::ResourceState::CopySource;
            cmdList->ResourceBarrier(1, &barrier);

            barrier.resource = albedoTex;
            barrier.fromState = RHI::ResourceState::Undefined;
            barrier.toState = RHI::ResourceState::CopyDestination;
            cmdList->ResourceBarrier(1, &barrier);

            barrier.resource = roughnessTex;
            barrier.fromState = RHI::ResourceState::Undefined;
            barrier.toState = RHI::ResourceState::CopyDestination;
            cmdList->ResourceBarrier(1, &barrier);

            barrier.resource = normalTex;
            barrier.fromState = RHI::ResourceState::Undefined;
            barrier.toState = RHI::ResourceState::CopyDestination;
            cmdList->ResourceBarrier(1, &barrier);

            // Albedo Copy
            RHI::BufferToTextureCopy copy{};
            copy.srcBuffer = stagingBuffer;
            copy.bufferOffset = 0;
            copy.dstTexture = albedoTex;
            copy.baseArrayLayer = 0;
            copy.layerCount = 1;
            copy.mipSlice = 0;
            cmdList->CopyTextureRegion(copy);

            // Roughness Copy
            copy.srcBuffer = stagingBuffer;
            copy.bufferOffset = roughnessOffset;
            copy.dstTexture = roughnessTex;
            cmdList->CopyTextureRegion(copy);

            // Normal copy
            copy.srcBuffer = stagingBuffer;
            copy.bufferOffset = normalOffset;
            copy.dstTexture = normalTex;
            cmdList->CopyTextureRegion(copy);
            
            barrier.resource = albedoTex;
            barrier.fromState = RHI::ResourceState::CopyDestination;
            barrier.toState = RHI::ResourceState::FragmentShaderResource;
            cmdList->ResourceBarrier(1, &barrier);

            barrier.resource = roughnessTex;
            barrier.fromState = RHI::ResourceState::CopyDestination;
            barrier.toState = RHI::ResourceState::FragmentShaderResource;
            cmdList->ResourceBarrier(1, &barrier);

            barrier.resource = normalTex;
            barrier.fromState = RHI::ResourceState::CopyDestination;
            barrier.toState = RHI::ResourceState::FragmentShaderResource;
            cmdList->ResourceBarrier(1, &barrier);
        }
        cmdList->End();

        queue->Execute(1, &cmdList, fence);
        fence->WaitForFence();

        // Cleanup
        RHI::gDynamicRHI->FreeCommandLists(1, &cmdList);
        delete stagingBuffer; stagingBuffer = nullptr;
        delete fence; fence = nullptr;

        auto timeTaken = ((f32)(clock() - prevTime)) / CLOCKS_PER_SEC;
    }

} // namespace CE::RPI
