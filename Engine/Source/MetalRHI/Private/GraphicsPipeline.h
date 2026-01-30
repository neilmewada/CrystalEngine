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
        
        std::array<id<MTLArgumentEncoder>, (int)RHI::SRGType::COUNT> vertEncodersBySetNumber;
        std::array<id<MTLArgumentEncoder>, (int)RHI::SRGType::COUNT> fragEncodersBySetNumber;
        
        MTLRenderPipelineDescriptor* pipelineDescriptor = nil;
        
        friend class ShaderResourceGroup;
    };
    
} // namespace CE::Metal
