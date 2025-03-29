//
// Created by Neel Mewada on 3/24/25.
//

#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{

    ComputePipeline::ComputePipeline(VulkanDevice* device, const RHI::ComputePipelineDescriptor& desc)
        : Pipeline(device, desc)
        , desc(desc)
    {
        pipelineType = PipelineStateType::Compute;

        SetupShaderStage();

        CompileInternal();
    }

    ComputePipeline::~ComputePipeline()
    {

    }

    void ComputePipeline::SetupShaderStage()
    {
        shaderStage = {};

        if (desc.shaderStages.NotEmpty())
        {
            auto& stage = desc.shaderStages[0];

            shaderEntryPoint = stage.entryPoint.GetString();

            shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStage.pName = shaderEntryPoint.GetCString();
            shaderStage.module = ((Vulkan::ShaderModule*)stage.shaderModule)->GetHandle();

            shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }


    void ComputePipeline::CompileInternal()
    {
        VkResult result = VK_SUCCESS;

        VkPipelineCacheCreateInfo cacheCI{};
        cacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        cacheCI.initialDataSize = 0;
        cacheCI.pInitialData = nullptr;

        if (pipelineCache == nullptr)
        {
            result = vkCreatePipelineCache(device->GetHandle(), &cacheCI, VULKAN_CPU_ALLOCATOR, &pipelineCache);
            if (result != VK_SUCCESS)
            {
                pipelineCache = nullptr;
            }
        }

        VkComputePipelineCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.layout = pipelineLayout;

        createInfo.stage = shaderStage;

        result = vkCreateComputePipelines(device->GetHandle(), pipelineCache, 1, &createInfo, nullptr, &pipeline);

        if (result != VK_SUCCESS)
        {
            CE_LOG(Error, All, "Failed to create Compute Pipeline: {}", name);
            return;
        }
    }
}
