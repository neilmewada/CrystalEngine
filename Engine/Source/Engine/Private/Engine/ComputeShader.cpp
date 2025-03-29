#include "Engine.h"

namespace CE
{

    CE::ComputeShader::ComputeShader()
    {

    }

    CE::ComputeShader::~ComputeShader()
    {
        for (auto& pair : computeShaders)
        {
            delete pair.second; pair.second = nullptr;
        }
        computeShaders.Clear();
    }

    CE::Name CE::ComputeShader::GetKernelName(int kernelIndex) const
    {
        if (kernelIndex <= kernelNames.GetSize() && kernelIndex >= 0)
        {
            return kernelNames[kernelIndex];
        }
        return "";
    }

    int CE::ComputeShader::FindKernelIndex(const CE::Name& kernelName) const
    {
        for (int i = 0; i < kernelNames.GetSize(); ++i)
        {
            if (kernelNames[i] == kernelName)
            {
                return i;
            }
        }

        return -1;
    }

    RPI::ComputeShader* CE::ComputeShader::GetRpiShader(int kernelIndex)
    {
        if (kernelIndex >= kernelNames.GetSize() || kernelIndex < 0 || kernelIndex >= kernels.GetSize())
            return nullptr;

        if (computeShaders[kernelIndex] != nullptr)
        {
            return computeShaders[kernelIndex];
        }

        RPI::ComputerShaderDescriptor descriptor{
            .shaderName = kernelNames[kernelIndex],
            .reflectionInfo = reflection,
            .entryPoint = kernelNames[kernelIndex]
        };

        descriptor.moduleDesc.name = kernelNames[kernelIndex].GetString();
        descriptor.moduleDesc.stage = RHI::ShaderStage::Compute;
        descriptor.moduleDesc.byteCode = kernels[kernelIndex].GetDataPtr();
        descriptor.moduleDesc.byteSize = kernels[kernelIndex].GetDataSize();
        descriptor.moduleDesc.debugName = descriptor.moduleDesc.name;

        computeShaders[kernelIndex] = new RPI::ComputeShader(descriptor);
        return computeShaders[kernelIndex];
    }

    
} // namespace CE

