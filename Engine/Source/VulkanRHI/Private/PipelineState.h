#pragma once

#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	class PipelineState;
	class GraphicsPipeline;

    class PipelineState : public RHI::PipelineState
    {
    public:
        PipelineState(Device* device, const RHI::GraphicsPipelineDescriptor& graphicsDesc);
    	PipelineState(Device* device, const RHI::ComputePipelineDescriptor& computeDesc);
        virtual ~PipelineState();

        virtual IPipelineLayout* GetPipelineLayout() override;

        inline Pipeline* GetPipeline() const { return pipeline; }

    protected:

        Device* device = nullptr;

		Pipeline* pipeline = nullptr;

		friend class GraphicsCommandList;
		friend class PipelineLayout;
		friend class GraphicsPipelineState;
    };
    
} // namespace CE::Editor
