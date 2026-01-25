#pragma once

namespace CE::Metal
{
    class Fence;

    class CommandList : public RHI::CommandList
    {
    public:
        CommandList(Device* device, id<MTLCommandQueue> mtlCommandQueue, RHI::CommandListType type);
        
        virtual ~CommandList();
        
        id<MTLCommandBuffer> GetMtlCommandBuffer() const { return mtlCommandBuffer; }
        
        void Begin() override;
        
        void End() override;
        
        void BeginRenderTarget(RHI::RenderTarget* renderTarget, RHI::RenderTargetBuffer* renderTargetBuffer, RHI::AttachmentClearValue* clearValuesPerAttachment) override;
        
        void EndRenderTarget() override;
        
        void RenderTargetNextSubPass() override;
        
        bool BeginRenderPass(RHI::RenderPass* renderPass, RHI::RenderPassFrameBuffer* frameBuffer, RHI::AttachmentClearValue* clearValuesPerAttachment) override;
        
        void RenderPassNextSubpass() override;
        
        void EndRenderPass() override;
        
        void ResourceBarrier(u32 count, ResourceBarrierDescriptor* barriers) override;
        
        void SetShaderResourceGroups(const ArrayView<RHI::ShaderResourceGroup* >& srgs) override;
        
        void ClearShaderResourceGroups() override;
        
        void SetRootConstants(u32 offset, u32 num32BitValues, const void* srcData) override;
        
        void SetViewports(u32 count, ViewportState* viewports) override;
        
        void SetScissors(u32 count, ScissorState* scissors) override;
        
        void CommitShaderResources() override;
        
        void BindPipelineState(RHI::PipelineState* pipelineState) override;
        
        void BindVertexBuffers(u32 firstInputSlot, u32 count, const RHI::VertexBufferView* bufferViews) override;
        
        void BindIndexBuffer(const RHI::IndexBufferView& bufferView) override;
        
        void DrawIndexed(const DrawIndexedArguments& args) override;
        
        void DrawLinear(const DrawLinearArguments& args) override;
        
        void Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) override;
        
        void CopyTextureRegion(const BufferToTextureCopy& region) override;
        
        void CopyTextureRegion(const TextureToBufferCopy& region) override;
        
        void CopyBufferRegion(const BufferCopy& copy) override;

    private:

        Device* device = nullptr;
        
        //Metal::RenderTarget* curRenderTarget = nullptr;
        Metal::RenderPass* curRenderPass = nullptr;
        int curSubpass = 0;
        
        id<MTLCommandQueue> mtlCommandQueue = nil;
        id<MTLCommandBuffer> mtlCommandBuffer = nil;
        id<MTLRenderCommandEncoder> mtlRenderEncoder = nil;
        id<MTLComputeCommandEncoder> mtlComputeEncoder = nil;
        
        RHI::IndexBufferView indexBufferView{};
        
        RHI::PipelineState* boundPipeline = nullptr;
        
        StaticArray<Metal::ShaderResourceGroup*, RHI::Limits::Pipeline::MaxShaderResourceGroupCount> boundSRGs{};
        Array<Metal::Buffer*> rootConstantBuffers;
        int currentRootConstantBufferIdx = -1;
        u32 rootConstantBufferByteSize = 0;
        
        Array<MTLViewport> viewportsArray;
        Array<MTLScissorRect> scissorsArray;
        
        friend class ShaderResourceGroup;
    };
    
} // namespace CE::Metal
