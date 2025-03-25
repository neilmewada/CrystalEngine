#pragma once
#include "RHI/PipelineState.h"

namespace CE::Vulkan
{

    class ComputePipeline : public Pipeline
    {
    public:

        ComputePipeline(VulkanDevice* device, const RHI::ComputePipelineDescriptor& desc);
        virtual ~ComputePipeline();

        VkPipelineBindPoint GetBindPoint() override { return VK_PIPELINE_BIND_POINT_COMPUTE; }

    private:

        void SetupShaderStage();

        void CompileInternal();

        RHI::ComputePipelineDescriptor desc{};

        String shaderEntryPoint;
        VkPipelineShaderStageCreateInfo shaderStage{};
    };

}
