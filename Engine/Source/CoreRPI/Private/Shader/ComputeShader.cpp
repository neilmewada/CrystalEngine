#include "CoreRPI.h"

namespace CE::RPI
{

    ComputeShader::ComputeShader(const ComputerShaderDescriptor& desc)
        : descriptor(desc)
    {
        shaderModule = RHI::gDynamicRHI->CreateShaderModule(desc.moduleDesc);

        RHI::ComputePipelineDescriptor pipelineDesc{};
        pipelineDesc.name = desc.shaderName;
        pipelineDesc.shaderStages = {
            ShaderStageDescriptor
            {
                .shaderModule = shaderModule,
                .entryPoint = desc.entryPoint
            }
        };
        pipelineDesc.srgLayouts = desc.reflectionInfo.srgLayouts;

        computePipelineState = RHI::gDynamicRHI->CreateComputePipeline(pipelineDesc);
    }

    ComputeShader::~ComputeShader()
    {
        if (computePipelineState)
        {
            RHI::gDynamicRHI->DestroyPipeline(computePipelineState);
            computePipelineState = nullptr;
        }

        if (shaderModule)
        {
            RHI::gDynamicRHI->DestroyShaderModule(shaderModule);
            shaderModule = nullptr;
        }
    }

    RHI::ShaderResourceGroupLayout ComputeShader::GetPassSrgLayout() const
    {
        for (const auto& srgLayout : descriptor.reflectionInfo.srgLayouts)
        {
            if (srgLayout.srgType == SRGType::PerPass)
            {
                return srgLayout;
            }
        }
        return {};
    }
}
