#pragma once

namespace CE::RPI
{
    struct ComputerShaderDescriptor
    {
        Name shaderName{};
        ShaderReflection reflectionInfo;
        RHI::ShaderModuleDescriptor moduleDesc{};
        Name entryPoint{};
    };

    class CORERPI_API ComputeShader final
    {
    public:

        ComputeShader(const ComputerShaderDescriptor& desc);

        ~ComputeShader();

        RHI::PipelineState* GetPipelineState() const { return computePipelineState; }

        RHI::ShaderResourceGroupLayout GetPassSrgLayout() const;

    private:

        ComputerShaderDescriptor descriptor;
        RHI::ShaderModule* shaderModule = nullptr;

        RHI::PipelineState* computePipelineState = nullptr;
    };

}
