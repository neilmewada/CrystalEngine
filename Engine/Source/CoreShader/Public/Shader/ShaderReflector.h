#pragma once

namespace CE
{
    
    class CORESHADER_API ShaderReflector
    {
    public:
        ShaderReflector();
        ~ShaderReflector();

		enum ErrorCode
		{
			ERR_Success = 0,
			ERR_InvalidFormat,
			ERR_FileNotFound,
			ERR_InvalidFile,
			ERR_FailedToLoadFile,
			ERR_FailedToReflect,
			ERR_InvalidArgs,
			ERR_UnsupportedPlatform,
			ERR_InvalidVertexInputNames,
		};

		ErrorCode ReflectSpirv(const void* byteCode, u32 byteSize, RHI::ShaderStage curStage, ShaderReflection& outReflection, const String& entryPoint);

		inline ErrorCode Reflect(ShaderBlobFormat shaderFormat, const void* byteCode, u32 byteSize, RHI::ShaderStage curStage, ShaderReflection& outReflection, const String& entryPoint)
		{
			if (shaderFormat == ShaderBlobFormat::Spirv)
			{
				return ReflectSpirv(byteCode, byteSize, curStage, outReflection, entryPoint);
			}
			return ERR_InvalidFormat;
		}

    private:

		

    };

} // namespace CE
