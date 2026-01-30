#pragma once

namespace CE::Metal
{

    class PipelineState;
    class GraphicsPipeline;

    class PipelineState : public RHI::PipelineState
    {
    public:
        
        PipelineState(Device* device, const RHI::GraphicsPipelineDescriptor& graphicsDesc);
        PipelineState(Device* device, const RHI::ComputePipelineDescriptor& computeDesc);
        virtual ~PipelineState();

        virtual IPipelineLayout* GetPipelineLayout() override { return pipeline; }

        inline Pipeline* GetPipeline() const { return pipeline; }
        
    private:
        
        Device* device = nullptr;

        Pipeline* pipeline = nullptr;
    };
    
} // namespace CE::Metal
