#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    PipelineState::PipelineState(Device* device, const RHI::GraphicsPipelineDescriptor& graphicsDesc)
    {
        this->graphicsDescriptor = graphicsDesc;
        pipeline = new GraphicsPipeline(device, graphicsDesc);

        pipelineType = PipelineStateType::Graphics;
    }

    PipelineState::PipelineState(Device* device, const RHI::ComputePipelineDescriptor& computeDesc)
    {
        this->computeDescriptor = computeDesc;
        pipeline = new ComputePipeline(device, computeDesc);

        pipelineType = PipelineStateType::Compute;
    }

    PipelineState::~PipelineState() noexcept
    {
        delete pipeline; pipeline = nullptr;
    }
        
} // namespace CE::Metal
