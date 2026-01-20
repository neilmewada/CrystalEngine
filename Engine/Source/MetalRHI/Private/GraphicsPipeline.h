#pragma once

namespace CE::Metal
{

    class GraphicsPipeline : public Pipeline
    {
    public:
        
        GraphicsPipeline(Device* device, const RHI::GraphicsPipelineDescriptor& desc);
        
        virtual ~GraphicsPipeline();
        
        id<MTLRenderPipelineState> GetMtlPipeline() const { return mtlPipeline; }
        
    private:
        
        void Create();
        
        void SetupShaderStages();
        
        void SetupColorAttachments();
        
        void SetupVertexInput();
        
        RHI::GraphicsPipelineDescriptor pipelineDesc{};
        
        // - ObjC -
        
        id<MTLRenderPipelineState> mtlPipeline = nil;
        
        MTLRenderPipelineDescriptor* pipelineDescriptor = nil;
    };
    
} // namespace CE::Metal
