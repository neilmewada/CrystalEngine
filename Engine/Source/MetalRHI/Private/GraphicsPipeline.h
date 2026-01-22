#pragma once

namespace CE::Metal
{

    class GraphicsPipeline : public Pipeline
    {
    public:
        
        GraphicsPipeline(Device* device, const RHI::GraphicsPipelineDescriptor& desc);
        
        virtual ~GraphicsPipeline();
        
        id<MTLRenderPipelineState> GetMtlPipeline() const { return mtlPipeline; }
        
        id<MTLDepthStencilState> GetDepthStencilState() const { return mtlDepthStencilState; }
        
        void SetupRenderEncoder(id<MTLRenderCommandEncoder> encoder);
        
    private:
        
        void Create();
        
        void SetupShaderStages();
        
        void SetupAttachments();
        
        void SetupVertexInput();
        
        void SetupDepthStencilState();
        
        RHI::GraphicsPipelineDescriptor pipelineDesc{};
        
        // - ObjC -
        
        id<MTLRenderPipelineState> mtlPipeline = nil;
        id<MTLDepthStencilState> mtlDepthStencilState = nil;
        
        MTLRenderPipelineDescriptor* pipelineDescriptor = nil;
    };
    
} // namespace CE::Metal
