#include "MetalRHIPrivate.h"

namespace CE::Metal
{

    ComputePipeline::ComputePipeline(Device* device, const RHI::ComputePipelineDescriptor& desc) : Pipeline(device, desc)
    {
        for (int i = 0; i < desc.shaderStages.GetSize(); i++)
        {
            if (desc.shaderStages[i].shaderModule->GetShaderStage() == RHI::ShaderStage::Compute)
            {
                NSString* entryPoint = [NSString stringWithCString:desc.shaderStages[i].entryPoint.GetCString()];
                auto shaderModule = (Metal::ShaderModule*)desc.shaderStages[i].shaderModule;
                id<MTLFunction> function = [shaderModule->GetMtlLibrary() newFunctionWithName:entryPoint];
                
                mtlPipeline = [device->GetHandle() newComputePipelineStateWithFunction:function error:nil];
                
                break;
            }
        }
    }

    ComputePipeline::~ComputePipeline()
    {
        
    }

} // namespace CE::Metal
