#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

    PipelineState::PipelineState(VulkanDevice* device, const RHI::GraphicsPipelineDescriptor& graphicsDesc)
		: device(device)
    {
		this->graphicsDescriptor = graphicsDesc;
		pipeline = new GraphicsPipeline(device, graphicsDesc);

    	pipelineType = PipelineStateType::Graphics;
    }

    PipelineState::PipelineState(VulkanDevice* device, const RHI::ComputePipelineDescriptor& computeDesc)
	    : device(device)
    {
    	this->computeDescriptor = computeDesc;
    	pipeline = new ComputePipeline(device, computeDesc);

    	pipelineType = PipelineStateType::Compute;
    }

    PipelineState::~PipelineState()
    {
		delete pipeline;
    }

	IPipelineLayout* PipelineState::GetPipelineLayout()
	{
		return pipeline;
	}

} // namespace CE
