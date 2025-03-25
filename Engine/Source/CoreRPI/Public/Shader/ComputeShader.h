#pragma once

namespace CE::RPI
{
    struct ComputerShaderDescriptor
    {
        Name shaderName{};
        ShaderReflection reflectionInfo;
        Array<ShaderTagEntry> tags{};
        RHI::ShaderModuleDescriptor moduleDesc{};
        Name entryPoint{};
    };

    class CORERPI_API ComputeShader final
    {
    public:

        ComputeShader(const ComputerShaderDescriptor& desc);

        ~ComputeShader();

    private:

        ComputerShaderDescriptor descriptor;
        RHI::ShaderModule* shaderModule = nullptr;

        RHI::PipelineState* computePipelineState = nullptr;
    };

}
