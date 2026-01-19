#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    CommandList::CommandList(Device* device, id<MTLCommandQueue> mtlCommandQueue, RHI::CommandListType type)
        : device(device), mtlCommandQueue(mtlCommandQueue)
    {
        this->commandListType = type;
    }

    CommandList::~CommandList()
    {
        
    }

    void CommandList::Begin()
    {
        mtlCommandBuffer = [mtlCommandQueue commandBuffer];
        
        curRenderTarget = nullptr;
    }

    void CommandList::End()
    {
        // Do nothing
    }

    void CommandList::BeginRenderTarget(RHI::RenderTarget* renderTarget, RHI::RenderTargetBuffer* renderTargetBuffer, RHI::AttachmentClearValue* clearValuesPerAttachment)
    {
        curRenderTarget = (Metal::RenderTarget*)renderTarget;
        
        curSubpass = 0;
        
        MTLRenderPassDescriptor* rpDesc = curRenderTarget->GetSubpass(curSubpass);
        
        mtlRenderEncoder = [mtlCommandBuffer renderCommandEncoderWithDescriptor:rpDesc];
    }

    void CommandList::EndRenderTarget()
    {
        [mtlRenderEncoder endEncoding];
        mtlRenderEncoder = nil;
    }

    void CommandList::RenderTargetNextSubPass()
    {
        curSubpass++;
        
        MTLRenderPassDescriptor* rpDesc = curRenderTarget->GetSubpass(curSubpass);
        
        mtlRenderEncoder = [mtlCommandBuffer renderCommandEncoderWithDescriptor:rpDesc];
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
        
    }

    void CommandList::SetRootConstants(u32 offset, u32 num32BitValues, const void* srcData)
    {
        
    }

    void CommandList::SetViewports(u32 count, ViewportState* viewports)
    {
        
    }

    void CommandList::SetScissors(u32 count, ScissorState* scissors)
    {
        
    }

    void CommandList::CommitShaderResources()
    {
        
    }

    void CommandList::BindPipelineState(RHI::PipelineState* pipelineState)
    {
        
    }

    void CommandList::BindVertexBuffers(u32 firstInputSlot, u32 count, const RHI::VertexBufferView* bufferViews)
    {
        
    }

    void CommandList::BindIndexBuffer(const RHI::IndexBufferView& bufferView)
    {
        
    }

    void CommandList::DrawIndexed(const DrawIndexedArguments& args)
    {
        
    }

    void CommandList::DrawLinear(const DrawLinearArguments& args)
    {
        
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
