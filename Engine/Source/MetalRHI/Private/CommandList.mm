#include "MetalRHIPrivate.h"

namespace CE::Metal
{
    
    inline static bool IsDepthFormat(RHI::Format format)
    {
        switch (format)
        {
        case RHI::Format::D16_UNORM_S8_UINT:
        case RHI::Format::D24_UNORM_S8_UINT:
        case RHI::Format::D32_SFLOAT_S8_UINT:
        case RHI::Format::D32_SFLOAT:
            return true;
        }
        return false;
    }

    inline static bool IsDepthStencilFormat(RHI::Format format)
    {
        switch (format)
        {
        case RHI::Format::D16_UNORM_S8_UINT:
        case RHI::Format::D24_UNORM_S8_UINT:
        case RHI::Format::D32_SFLOAT_S8_UINT:
            return true;
        }
        return false;
    }

    CommandList::CommandList(Device* device, id<MTLCommandQueue> mtlCommandQueue, RHI::CommandListType type)
        : device(device), mtlCommandQueue(mtlCommandQueue)
    {
        this->commandListType = type;
        
        RHI::BufferDescriptor bufferDesc{};
        bufferDesc.name = "RootConstants";
        bufferDesc.bufferSize = device->GetDeviceLimits()->GetMaxRootConstantByteSize();
        bufferDesc.defaultHeapType = RHI::MemoryHeapType::Upload;
        bufferDesc.bindFlags = RHI::BufferBindFlags::ConstantBuffer;
        bufferDesc.structureByteStride = sizeof(u32);
        
        rootConstantBuffer = new Metal::Buffer(device, bufferDesc);
    }

    CommandList::~CommandList()
    {
        delete rootConstantBuffer; rootConstantBuffer = nullptr;
        
        mtlCommandQueue = nil;
        mtlCommandBuffer = nil;
    }

    void CommandList::Begin()
    {
        mtlCommandBuffer = [mtlCommandQueue commandBuffer];
        
        curRenderPass = nil;
        curSubpass = 0;
    }

    void CommandList::End()
    {
        // Do nothing
    }

    void CommandList::BeginRenderTarget(RHI::RenderTarget* renderTarget, RHI::RenderTargetBuffer* renderTargetBuffer, RHI::AttachmentClearValue* clearValuesPerAttachment)
    {
        
    }

    void CommandList::EndRenderTarget()
    {
    }

    void CommandList::RenderTargetNextSubPass()
    {
        
    }
    
    bool CommandList::BeginRenderPass(RHI::RenderPass* renderPass, RHI::RenderPassFrameBuffer* rhiFrameBuffer, RHI::AttachmentClearValue* clearValuesPerAttachment)
    {
        curRenderPass = (Metal::RenderPass*)renderPass;
        
        curSubpass = 0;
        
        MTLRenderPassDescriptor* originalRpDesc = curRenderPass->GetSubpass(curSubpass);
        
        MTLRenderPassDescriptor* rpDesc = [originalRpDesc copy];
        defer(&) {
            [rpDesc release];
        };
        
        auto frameBuffer = (Metal::RenderPassFrameBuffer*)rhiFrameBuffer;
        
        const auto& rpLayout = curRenderPass->GetRenderPassLayout();
        const auto& subpassLayout = rpLayout.subpasses[curSubpass];
        bool hasResolve = (subpassLayout.resolveAttachments.GetSize() == subpassLayout.colorAttachments.GetSize());
        
        u32 frameIndex = this->currentFrameIndex;
        
        for (int i = 0; i < subpassLayout.colorAttachments.GetSize(); i++)
        {
            u32 colorAttachmentIdx = subpassLayout.colorAttachments[i];
            const RHI::RenderPassAttachmentLayout& colorAttachmentLayout = rpLayout.attachmentLayouts[colorAttachmentIdx];
            const RHI::RenderPassFrameAttachment& colorAttachment = frameBuffer->GetAttachment(colorAttachmentIdx);
            Vec4 clearColor = clearValuesPerAttachment[colorAttachmentIdx].clearValue;
            
            if (RHI::SwapChain* swapChainAttachment = colorAttachment.GetSwapChain())
            {
                Metal::SwapChain* metalSwapChain = (Metal::SwapChain*)swapChainAttachment;
                if (metalSwapChain->GetCurrentDrawable() == nil)
                {
                    CE_LOG(Error, All, "BeginRenderPass(): SwapChain passed as a RenderPassFrameAttachment (color) without acquiring an image.");
                    return false;
                }
                
                rpDesc.colorAttachments[i].texture = [metalSwapChain->GetCurrentDrawable() texture];
                rpDesc.colorAttachments[i].clearColor = MTLClearColorMake(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
            }
            else if (RHI::TextureView* textureViewAttachment = colorAttachment.GetTextureView(frameIndex))
            {
                Metal::TextureView* textureView = (Metal::TextureView*)textureViewAttachment;
                
                rpDesc.colorAttachments[i].texture = textureView->GetMtlTextureView();
                rpDesc.colorAttachments[i].clearColor = MTLClearColorMake(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
            }
            else if (RHI::Texture* textureAttachment = colorAttachment.GetTexture(frameIndex))
            {
                Metal::Texture* texture = (Metal::Texture*)textureAttachment;
                
                rpDesc.colorAttachments[i].texture = texture->GetMtlTexture();
                rpDesc.colorAttachments[i].clearColor = MTLClearColorMake(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
            }
            else
            {
                CE_LOG(Error, All, "BeginRenderPass(): Invalid RenderPassFrameAttachment (color) at index {} in RenderPassFrameBuffer.", colorAttachmentIdx);
                return false;
            }
            
            if (hasResolve)
            {
                u32 resolveAttachmentIdx = subpassLayout.resolveAttachments[i];
                const RHI::RenderPassAttachmentLayout& resolveAttachmentLayout = rpLayout.attachmentLayouts[resolveAttachmentIdx];
                const RHI::RenderPassFrameAttachment& resolveAttachment = frameBuffer->GetAttachment(resolveAttachmentIdx);
                
                if (RHI::SwapChain* swapChainAttachment = resolveAttachment.GetSwapChain())
                {
                    Metal::SwapChain* metalSwapChain = (Metal::SwapChain*)swapChainAttachment;
                    if (metalSwapChain->GetCurrentDrawable() == nil)
                    {
                        CE_LOG(Error, All, "BeginRenderPass(): SwapChain passed as a RenderPassFrameAttachment (resolve) without acquiring an image.");
                        return false;
                    }
                    
                    rpDesc.colorAttachments[i].resolveTexture = [metalSwapChain->GetCurrentDrawable() texture];
                }
                else if (RHI::TextureView* textureViewAttachment = resolveAttachment.GetTextureView(frameIndex))
                {
                    Metal::TextureView* textureView = (Metal::TextureView*)textureViewAttachment;
                    
                    rpDesc.colorAttachments[i].resolveTexture = textureView->GetMtlTextureView();
                }
                else if (RHI::Texture* textureAttachment = resolveAttachment.GetTexture(frameIndex))
                {
                    Metal::Texture* texture = (Metal::Texture*)textureAttachment;
                    
                    rpDesc.colorAttachments[i].resolveTexture = texture->GetMtlTexture();
                }
                else
                {
                    CE_LOG(Error, All, "BeginRenderPass(): Invalid RenderPassFrameAttachment (resolve) at index {} in RenderPassFrameBuffer.", colorAttachmentIdx);
                    return false;
                }
            }
        }
        
        if (subpassLayout.depthStencilAttachment.GetSize() > 0)
        {
            u32 depthStencilAttachmentIdx = subpassLayout.depthStencilAttachment[0];
            const RHI::RenderPassAttachmentLayout& depthStencilAttachmentLayout = rpLayout.attachmentLayouts[depthStencilAttachmentIdx];
            const RHI::RenderPassFrameAttachment& depthStencilAttachment = frameBuffer->GetAttachment(depthStencilAttachmentIdx);
            f32 depthClearValue = clearValuesPerAttachment[depthStencilAttachmentIdx].clearValueDepth;
            u8 stencilClearValue = clearValuesPerAttachment[depthStencilAttachmentIdx].clearValueStencil;
            
            bool hasStencil = IsDepthStencilFormat(depthStencilAttachmentLayout.format);
            
            if (RHI::TextureView* textureViewAttachment = depthStencilAttachment.GetTextureView(frameIndex))
            {
                Metal::TextureView* textureView = (Metal::TextureView*)textureViewAttachment;
                
                rpDesc.depthAttachment.texture = textureView->GetMtlTextureView();
                rpDesc.depthAttachment.clearDepth = depthClearValue;
                
                if (hasStencil)
                {
                    rpDesc.stencilAttachment.texture = textureView->GetMtlTextureView();
                    rpDesc.stencilAttachment.clearStencil = stencilClearValue;
                }
            }
            else if (RHI::Texture* textureAttachment = depthStencilAttachment.GetTexture(frameIndex))
            {
                Metal::Texture* texture = (Metal::Texture*)textureAttachment;
                
                rpDesc.depthAttachment.texture = texture->GetMtlTexture();
                rpDesc.depthAttachment.clearDepth = depthClearValue;
                
                if (hasStencil)
                {
                    rpDesc.stencilAttachment.texture = texture->GetMtlTexture();
                    rpDesc.stencilAttachment.clearStencil = stencilClearValue;
                }
            }
            else
            {
                CE_LOG(Error, All, "BeginRenderPass(): Invalid RenderPassFrameAttachment (depth) at index {} in RenderPassFrameBuffer.", depthStencilAttachmentIdx);
                return false;
            }
        }
        
        mtlRenderEncoder = [mtlCommandBuffer renderCommandEncoderWithDescriptor:rpDesc];
        
        return mtlRenderEncoder != nil;
    }
    
    void CommandList::RenderPassNextSubpass()
    {
        curSubpass++;
        
        // FIXME: Subpasses not supported on either Metal or Vulkan yet.
    }
    
    void CommandList::EndRenderPass()
    {
        [mtlRenderEncoder endEncoding];
        
        mtlRenderEncoder = nil;
    }

    void CommandList::ResourceBarrier(u32 count, ResourceBarrierDescriptor* barriers)
    {
        if (count == 0)
            return;

        for (int i = 0; i < count; i++)
        {
            const RHI::ResourceBarrierDescriptor& barrierInfo = barriers[i];
            if (barrierInfo.resource == nullptr)
                continue;
            
            MTLResourceUsage usage = 0;
            MTLRenderStages stages = MTLRenderStageVertex | MTLRenderStageFragment;
            
            switch (barrierInfo.toState)
            {
                case RHI::ResourceState::General:
                    usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                    break;
                case RHI::ResourceState::CopyDestination:
                    usage = MTLResourceUsageWrite;
                    break;
                case RHI::ResourceState::CopySource:
                    usage = MTLResourceUsageRead;
                    break;
                case RHI::ResourceState::DepthWrite:
                    usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                    break;
                case RHI::ResourceState::DepthRead:
                    usage = MTLResourceUsageRead;
                    break;
                case RHI::ResourceState::FragmentShaderResource:
                    usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                    stages = MTLRenderStageFragment;
                    break;
                case RHI::ResourceState::NonFragmentShaderResource:
                    usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                    stages = MTLRenderStageVertex;
                    break;
                case RHI::ResourceState::ColorOutput:
                    usage = MTLResourceUsageWrite;
                    stages = MTLRenderStageFragment;
                    break;
                case RHI::ResourceState::VertexBuffer:
                    usage = MTLResourceUsageRead;
                    stages = MTLRenderStageVertex;
                    break;
                case RHI::ResourceState::IndexBuffer:
                    usage = MTLResourceUsageRead;
                    stages = MTLRenderStageVertex;
                    break;
                case RHI::ResourceState::ConstantBuffer:
                    usage = MTLResourceUsageRead;
                    break;
                case RHI::ResourceState::Present:
                    usage = MTLResourceUsageRead;
                    break;
                case RHI::ResourceState::ShaderWrite:
                    usage = MTLResourceUsageRead | MTLResourceUsageWrite;
                    break;
                case RHI::ResourceState::BlitSource:
                    usage = MTLResourceUsageRead;
                    break;
                case RHI::ResourceState::BlitDestination:
                    usage = MTLResourceUsageWrite;
                    break;
            }
            
            RHI::DeviceObjectType resourceType = barrierInfo.resource->GetDeviceObjectType();
            if (resourceType == RHI::DeviceObjectType::Texture)
            {
                Metal::Texture* texture = (Metal::Texture*)barrierInfo.resource;
                
                if (mtlRenderEncoder != nil && texture != nullptr)
                {
                    [mtlRenderEncoder useResource:texture->GetMtlTexture() usage:usage stages:stages];
                }
                else if (mtlComputeEncoder != nil && texture != nullptr)
                {
                    [mtlComputeEncoder useResource:texture->GetMtlTexture() usage:usage];
                }
            }
            else if (resourceType == RHI::DeviceObjectType::Buffer)
            {
                Metal::Buffer* buffer = (Metal::Buffer*)barrierInfo.resource;
                
                if (mtlRenderEncoder != nil && buffer != nullptr)
                {
                    [mtlRenderEncoder useResource:buffer->GetMtlBuffer() usage:usage stages:stages];
                }
                else if (mtlComputeEncoder != nil && buffer != nullptr)
                {
                    [mtlComputeEncoder useResource:buffer->GetMtlBuffer() usage:usage];
                }
            }
        }
    }

    void CommandList::SetShaderResourceGroups(const ArrayView<RHI::ShaderResourceGroup*>& srgs)
    {
        for (auto rhiSrg : srgs)
        {
            auto srg = (Metal::ShaderResourceGroup*)rhiSrg;
            boundSRGs[(int)srg->GetSRGType()] = srg;
        }
    }

    void CommandList::ClearShaderResourceGroups()
    {
        for (int i = 0; i < RHI::Limits::Pipeline::MaxShaderResourceGroupCount; i++)
        {
            boundSRGs[i] = nullptr;
        }
    }

    void CommandList::SetViewports(u32 count, ViewportState* viewports)
    {
        if (count == 0)
            return;
        
        if (viewportsArray.GetSize() < count)
        {
            viewportsArray.Resize(count);
        }
        
        for (int i = 0; i < count; i++)
        {
            MTLViewport viewport;
            viewport.width = viewports[i].width;
            viewport.height = viewports[i].height;
            viewport.originX = viewports[i].x;
            viewport.originY = viewports[i].y;
            viewport.znear = viewports[i].minDepth;
            viewport.zfar = viewports[i].maxDepth;
            
            if (count == 1)
            {
                [mtlRenderEncoder setViewport:viewport];
                return;
            }
            
            viewportsArray[i] = viewport;
        }
        
        [mtlRenderEncoder setViewports:viewportsArray.GetData() count:count];
    }

    void CommandList::SetScissors(u32 count, ScissorState* scissors)
    {
        if (count == 0)
            return;
        
        if (scissorsArray.GetSize() < count)
        {
            scissorsArray.Resize(count);
        }
        
        for (int i = 0; i < count; i++)
        {
            MTLScissorRect rect;
            rect.x = scissors[i].x;
            rect.y = scissors[i].y;
            rect.width = scissors[i].width;
            rect.height = scissors[i].height;
            
            if (count == 1)
            {
                [mtlRenderEncoder setScissorRect:rect];
                return;
            }
            
            scissorsArray[i] = rect;
        }
        
        [mtlRenderEncoder setScissorRects:scissorsArray.GetData() count:count];
    }

    void CommandList::CommitShaderResources()
    {
        for (int i = 0; i < RHI::Limits::Pipeline::MaxShaderResourceGroupCount; i++)
        {
            Metal::ShaderResourceGroup* srg = boundSRGs[i];
            if (!srg)
                continue;
            
            //id<MTLBuffer> argumentBuffer = srg->GetArgumentBuffer(currentFrameIndex);
            srg->CommitResources(this);
            
            if (mtlRenderEncoder != nil)
            {
                //[mtlRenderEncoder setVertexBuffer:argumentBuffer offset:0 atIndex:(int)srg->GetSRGType()];
                //[mtlRenderEncoder setFragmentBuffer:argumentBuffer offset:0 atIndex:(int)srg->GetSRGType()];
                srg->MtlUseResources(mtlRenderEncoder, currentFrameIndex);
            }
            else if (mtlComputeEncoder != nil)
            {
                //[mtlComputeEncoder setBuffer:argumentBuffer offset:0 atIndex:(int)srg->GetSRGType()];
                
                srg->MtlUseResources(mtlComputeEncoder, currentFrameIndex);
            }
        }
        
        int rootConstantBufferIndex = (int)RHI::SRGType::RootConstant;
        
        if (mtlRenderEncoder != nil)
        {
            [mtlRenderEncoder setVertexBuffer:rootConstantBuffer->GetMtlBuffer() offset:0 atIndex:rootConstantBufferIndex];
            [mtlRenderEncoder setFragmentBuffer:rootConstantBuffer->GetMtlBuffer() offset:0 atIndex:rootConstantBufferIndex];
        }
        else if (mtlComputeEncoder != nil)
        {
            [mtlComputeEncoder setBuffer:rootConstantBuffer->GetMtlBuffer() offset:0 atIndex:rootConstantBufferIndex];
        }
    }
    
    void CommandList::SetRootConstants(u32 offset, u32 num32BitValues, const void* srcData)
    {
        rootConstantBuffer->UploadData({ .dataSize = sizeof(uint32_t) * num32BitValues, .data = srcData });
        
        int rootConstantBufferIndex = (int)RHI::SRGType::RootConstant;
        
        if (mtlRenderEncoder != nil)
        {
            [mtlRenderEncoder setVertexBuffer:rootConstantBuffer->GetMtlBuffer() offset:0 atIndex:rootConstantBufferIndex];
            [mtlRenderEncoder setFragmentBuffer:rootConstantBuffer->GetMtlBuffer() offset:0 atIndex:rootConstantBufferIndex];
        }
        else if (mtlComputeEncoder != nil)
        {
            [mtlComputeEncoder setBuffer:rootConstantBuffer->GetMtlBuffer() offset:0 atIndex:rootConstantBufferIndex];
        }
    }

    void CommandList::BindPipelineState(RHI::PipelineState* pipelineState)
    {
        boundPipeline = pipelineState;
        
        if (!boundPipeline)
            return;
        
        auto metalPipelineState = (Metal::PipelineState*)boundPipeline;
        
        if (boundPipeline->GetPipelineType() == RHI::PipelineStateType::Graphics)
        {
            auto graphicsPipeline = (Metal::GraphicsPipeline*)metalPipelineState->GetPipeline();
            
            [mtlRenderEncoder setRenderPipelineState:graphicsPipeline->GetMtlPipeline()];
            
            graphicsPipeline->SetupRenderEncoder(mtlRenderEncoder);
        }
        else if (boundPipeline->GetPipelineType() == RHI::PipelineStateType::Compute)
        {
            auto computePipeline = (Metal::ComputePipeline*)metalPipelineState->GetPipeline();
            
            [mtlComputeEncoder setComputePipelineState:computePipeline->GetMtlPipeline()];
            
            // TODO: Compute pipeline
        }
    }

    void CommandList::BindVertexBuffers(u32 firstInputSlot, u32 count, const RHI::VertexBufferView* bufferViews)
    {
        if (!mtlRenderEncoder)
            return;
            
        for (int i = 0; i < count; i++)
        {
            RHI:VertexBufferView bufferView = bufferViews[i];
            Metal::Buffer* buffer = (Metal::Buffer*)bufferView.GetBuffer();
            
            [mtlRenderEncoder setVertexBuffer:buffer->GetMtlBuffer() offset:bufferView.GetByteOffset() atIndex:((int)RHI::SRGType::COUNT + firstInputSlot + i)];
        }
    }

    void CommandList::BindIndexBuffer(const RHI::IndexBufferView& bufferView)
    {
        this->indexBufferView = bufferView;
    }

    void CommandList::DrawIndexed(const DrawIndexedArguments& args)
    {
        if (!mtlRenderEncoder)
            return;
        
        MTLIndexType indexType = indexBufferView.GetIndexFormat() == RHI::IndexFormat::Uint32 ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
        NSUInteger indexStride = indexType == MTLIndexTypeUInt32 ? 4 : 2;
        Metal::Buffer* indexBuffer = (Metal::Buffer*)indexBufferView.GetBuffer();
        
        [mtlRenderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                     indexCount:args.indexCount
                                      indexType:indexType
                                    indexBuffer:indexBuffer->GetMtlBuffer()
                              indexBufferOffset:indexBufferView.GetByteOffset() + args.firstIndex * indexStride
                                  instanceCount:args.instanceCount
                                     baseVertex:args.vertexOffset
                                   baseInstance:args.firstInstance];
    }

    void CommandList::DrawLinear(const DrawLinearArguments& args)
    {
        if (!mtlRenderEncoder)
            return;
        
        [mtlRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                             vertexStart:args.vertexOffset
                             vertexCount:args.vertexCount
                           instanceCount:args.instanceCount
                            baseInstance:args.firstInstance];
    }

    void CommandList::Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
    {
        
    }

    void CommandList::CopyTextureRegion(const BufferToTextureCopy& region)
    {
        id<MTLBlitCommandEncoder> blitEncoder = [mtlCommandBuffer blitCommandEncoder];
        
        auto srcBuffer = (Metal::Buffer*)region.srcBuffer;
        u32 width = region.dstTexture->GetWidth();
        auto texture = (Metal::Texture*)region.dstTexture;
        
        u64 bytesPerRow = texture->GetWidth(region.mipSlice) * texture->GetBitsPerPixel() / 8;
        u64 bytesPerImage = bytesPerRow * texture->GetHeight(region.mipSlice);
        
        for (int layer = region.baseArrayLayer; layer < region.baseArrayLayer + region.layerCount; layer++)
        {
            [blitEncoder copyFromBuffer:srcBuffer->GetMtlBuffer()
                           sourceOffset:region.bufferOffset
                      sourceBytesPerRow:bytesPerRow
                    sourceBytesPerImage:bytesPerImage
                             sourceSize:MTLSizeMake(texture->GetWidth(region.mipSlice), texture->GetHeight(region.mipSlice), texture->GetDepth(region.mipSlice))
                              toTexture:texture->GetMtlTexture()
                       destinationSlice:region.mipSlice
                       destinationLevel:layer
                      destinationOrigin:MTLOriginMake(0, 0, 0)];
        }
        
        [blitEncoder endEncoding];
    }

    void CommandList::CopyTextureRegion(const TextureToBufferCopy& region)
    {
        id<MTLBlitCommandEncoder> blitEncoder = [mtlCommandBuffer blitCommandEncoder];
        
        auto srcTexture = (Metal::Texture*)region.srcTexture;
        auto dstBuffer = (Metal::Buffer*)region.dstBuffer;
        
        u64 bytesPerRow = srcTexture->GetWidth(region.mipSlice) * srcTexture->GetBitsPerPixel() / 8;
        u64 bytesPerImage = bytesPerRow * srcTexture->GetHeight(region.mipSlice);
        
        for (int layer = region.baseArrayLayer; layer < region.baseArrayLayer + region.layerCount; layer++)
        {
            [blitEncoder copyFromTexture:srcTexture->GetMtlTexture()
                             sourceSlice:region.mipSlice
                             sourceLevel:layer
                            sourceOrigin:MTLOriginMake(0, 0, 0)
                              sourceSize:MTLSizeMake(srcTexture->GetWidth(region.mipSlice), srcTexture->GetHeight(region.mipSlice), srcTexture->GetDepth(region.mipSlice))
                                toBuffer:dstBuffer->GetMtlBuffer()
                       destinationOffset:region.bufferOffset
                  destinationBytesPerRow:bytesPerRow
                destinationBytesPerImage:bytesPerImage];
        }
        
        [blitEncoder endEncoding];
    }

    void CommandList::CopyBufferRegion(const BufferCopy& copy)
    {
        id<MTLBlitCommandEncoder> blitEncoder = [mtlCommandBuffer blitCommandEncoder];
        
        auto srcBuffer = (Metal::Buffer*)copy.srcBuffer;
        auto dstBuffer = (Metal::Buffer*)copy.dstBuffer;
        
        [blitEncoder copyFromBuffer:srcBuffer->GetMtlBuffer()
                       sourceOffset:copy.srcOffset
                           toBuffer:dstBuffer->GetMtlBuffer()
                  destinationOffset:copy.dstOffset
                               size:copy.totalByteSize];
        
        [blitEncoder endEncoding];
    }
    

} // namespace CE::Metal
