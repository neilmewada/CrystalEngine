
#include "CoreShader.h"

#include <PAL/Common/PlatformSystemIncludes.h>

#if PLATFORM_WINDOWS
#else
#   define __EMULATE_UUID
#endif

#if PLATFORM_DESKTOP && !CE_STANDALONE
#include <spirv_cross/spirv_msl.hpp>
#include "spirv_cross/spirv_reflect.hpp"
#include "spirv_cross/spirv_parser.hpp"
#include <spirv-tools/libspirv.hpp>

#undef SIZE_T
#include <dxc/dxcapi.h>

#define SHADER_COMPILATION_SUPPORTED 1
#endif

#include <locale>
#include <codecvt>
#include <string>

namespace CE
{

	static std::wstring ToWString(const std::string& stringToConvert)
	{
		std::wstring wideString = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(stringToConvert);
		return wideString;
	}

	static std::wstring ToWString(const String& string)
	{
		std::string stringToConvert = string.ToStdString();
		std::wstring wideString =
			std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(stringToConvert);
		return wideString;
	}

    static spv::ExecutionModel ShaderStageToExecutionModel(RHI::ShaderStage stage)
    {
        switch (stage) {
            case RHI::ShaderStage::Vertex:
                return spv::ExecutionModelVertex;
            case RHI::ShaderStage::Fragment:
                return spv::ExecutionModelFragment;
            case RHI::ShaderStage::Compute:
                return spv::ExecutionModelKernel;
            case RHI::ShaderStage::Geometry:
                return spv::ExecutionModelGeometry;
            default:
                return spv::ExecutionModelMax;
        }
    }
	
	struct ShaderCompiler::Impl
	{
		CComPtr<IDxcUtils> utils;
		CComPtr<IDxcCompiler3> compiler;
		CComPtr<IDxcIncludeHandler> includeHandler;

		~Impl()
		{
			utils.Release();
			compiler.Release();
			includeHandler.Release();
		}
	};

	ShaderCompiler::ShaderCompiler()
	{
		impl = new Impl();

		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&impl->utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&impl->compiler));
		
		impl->utils->CreateDefaultIncludeHandler(&impl->includeHandler);
	}

	ShaderCompiler::~ShaderCompiler()
	{
		delete impl;
	}

    ShaderCompiler::ErrorCode ShaderCompiler::BuildMSL(const void* data, u32 dataSize, const ShaderBuildConfig& buildConfig, BinaryBlob& outByteCode, Array<std::wstring>& extraArgs, ShaderReflection* outReflection)
    {
#if !SHADER_COMPILATION_SUPPORTED
        return ERR_UnsupportedPlatform;
#else
        
        HRESULT status = 0;
        ShaderBuildConfig& config = const_cast<ShaderBuildConfig&>(buildConfig);

        CComPtr<IDxcBlobEncoding> source = nullptr;

        status = impl->utils->CreateBlob(data, dataSize, DXC_CP_UTF8, &source);

        if (!SUCCEEDED(status))
        {
            this->errorMessage = "Failed to load source file";
            return ERR_FailedToLoadFile;
        }

        DxcBuffer buffer;
        buffer.Ptr = source->GetBufferPointer();
        buffer.Size = source->GetBufferSize();
        buffer.Encoding = DXC_CP_UTF8;
        
        BinaryBlob spirvCode;

        ErrorCode result = BuildSpirv(buffer, config, spirvCode, extraArgs);
        if (result != ERR_Success)
        {
            return result;
        }
        
        if (outReflection != nullptr)
        {
            ShaderReflector reflector{};
            reflector.ReflectSpirv(spirvCode.GetDataPtr(), spirvCode.GetDataSize(), buildConfig.stage, *outReflection, buildConfig.entry);
            
            for (int i = 0; i < outReflection->srgLayouts.GetSize(); i++)
            {
                for (int j = 0; j < outReflection->srgLayouts[i].variables.GetSize(); j++)
                {
                    //outReflection->srgLayouts[i].variables[j].bindingSlot = j;
                }
            }
        }
        
        spirv_cross::CompilerMSL compiler((u32*)spirvCode.GetDataPtr(), spirvCode.GetDataSize() / 4);
        
        spirv_cross::CompilerMSL::Options mslOptions;
        mslOptions.platform = spirv_cross::CompilerMSL::Options::Platform::macOS;
        mslOptions.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(2, 3);
        mslOptions.argument_buffers = true;
        mslOptions.force_active_argument_buffer_resources = true;
        mslOptions.pad_argument_buffer_resources = true;
        compiler.set_msl_options(mslOptions);
        
        if (outReflection != nullptr)
        {
            for (int i = 0; i < outReflection->srgLayouts.GetSize(); i++)
            {
                for (int j = 0; j < outReflection->srgLayouts[i].variables.GetSize(); j++)
                {
                    const auto& variable = outReflection->srgLayouts[i].variables[j];
                    
                    spirv_cross::MSLResourceBinding mslBinding{};
                    mslBinding.desc_set = (uint32_t)outReflection->srgLayouts[i].srgType;
                    mslBinding.binding = variable.bindingSlot;
                    mslBinding.stage = ShaderStageToExecutionModel(buildConfig.stage);
                    mslBinding.count = 1;
                    
                    switch (variable.type)
                    {
                        case RHI::ShaderResourceType::None:
                            continue;
                        case RHI::ShaderResourceType::ConstantBuffer:
                        case RHI::ShaderResourceType::StructuredBuffer:
                        case RHI::ShaderResourceType::RWStructuredBuffer:
                            mslBinding.basetype = spirv_cross::SPIRType::Void;
                            mslBinding.msl_buffer = variable.bindingSlot;
                            break;
                        case RHI::ShaderResourceType::Texture1D:
                        case RHI::ShaderResourceType::Texture2D:
                        case RHI::ShaderResourceType::Texture2DArray:
                        case RHI::ShaderResourceType::Texture3D:
                        case RHI::ShaderResourceType::TextureCube:
                        case RHI::ShaderResourceType::RWTexture2D:
                        case RHI::ShaderResourceType::RWTexture3D:
                        case RHI::ShaderResourceType::RWTexture2DArray:
                        case RHI::ShaderResourceType::SubpassInput:
                            mslBinding.basetype = spirv_cross::SPIRType::Image;
                            mslBinding.msl_texture = variable.bindingSlot;
                            break;
                        case RHI::ShaderResourceType::SamplerState:
                            mslBinding.basetype = spirv_cross::SPIRType::Sampler;
                            mslBinding.msl_sampler = variable.bindingSlot;
                            break;
                    }
                    
                    try
                    {
                        compiler.add_msl_resource_binding(mslBinding);
                    }
                    catch (const std::exception& e)
                    {
                        const char* msg = e.what();
                        String::IsAlphabet('a');
                    }
                }
            }
        }
        
        try
        {
            std::string resultStr = compiler.compile();
            if (resultStr.empty())
                return ERR_CompilationFailure;
            
            outByteCode.LoadData(resultStr.c_str(), resultStr.length() + 1); // +1 for null-terminator
        }
        catch (const std::exception& e)
        {
            const char* msg = e.what();
            
            return ERR_InvalidBuildFormat;
        }
        
        return ERR_Success;
#endif
    }

    ShaderCompiler::ErrorCode ShaderCompiler::CompileMSL(const IO::Path& hlslPath, ShaderCompilationInfo& config)
    {
        if (!hlslPath.Exists())
            return ERR_FileNotFound;
        if (hlslPath.IsDirectory())
            return ERR_InvalidFile;
        
        HRESULT status;

        CComPtr<IDxcBlobEncoding> source = nullptr;
        std::wstring str = ToWString(hlslPath.GetString());
        std::wstring fileNameStr = ToWString(hlslPath.GetFileName().GetString());

        status = impl->utils->LoadFile(str.data(), nullptr, &source);
        
        if (!SUCCEEDED(status))
        {
            this->errorMessage = "Failed to load source file";
            return ERR_FailedToLoadFile;
        }

        DxcBuffer buffer;
        buffer.Ptr = source->GetBufferPointer();
        buffer.Size = source->GetBufferSize();
        buffer.Encoding = DXC_CP_UTF8;
        
        return CompileMSL(buffer, config);
    }

    ShaderCompiler::ErrorCode ShaderCompiler::CompileMSL(const void* data, u32 dataSize, ShaderCompilationInfo& config)
    {
        HRESULT status = 0;

        CComPtr<IDxcBlobEncoding> source = nullptr;

        status = impl->utils->CreateBlob(data, dataSize, DXC_CP_UTF8, &source);

        if (!SUCCEEDED(status))
        {
            this->errorMessage = "Failed to load source file";
            return ERR_FailedToLoadFile;
        }

        DxcBuffer buffer;
        buffer.Ptr = source->GetBufferPointer();
        buffer.Size = source->GetBufferSize();
        buffer.Encoding = DXC_CP_UTF8;
        
        return CompileMSL(buffer, config);
    }

    ShaderCompiler::ErrorCode ShaderCompiler::CompileSpirv(const IO::Path& hlslPath, ShaderCompilationInfo& config)
    {
        if (!hlslPath.Exists())
            return ERR_FileNotFound;
        if (hlslPath.IsDirectory())
            return ERR_InvalidFile;

        HRESULT status;

        CComPtr<IDxcBlobEncoding> source = nullptr;
        std::wstring str = ToWString(hlslPath.GetString());
        std::wstring fileNameStr = ToWString(hlslPath.GetFileName().GetString());

        status = impl->utils->LoadFile(str.data(), nullptr, &source);

        if (!SUCCEEDED(status))
        {
            this->errorMessage = "Failed to load source file";
            return ERR_FailedToLoadFile;
        }

        DxcBuffer buffer;
        buffer.Ptr = source->GetBufferPointer();
        buffer.Size = source->GetBufferSize();
        buffer.Encoding = DXC_CP_UTF8;

        return CompileSpirv(buffer, config);
    }

    ShaderCompiler::ErrorCode ShaderCompiler::CompileSpirv(const void* data, u32 dataSize, ShaderCompilationInfo& config)
    {
        HRESULT status = 0;

        CComPtr<IDxcBlobEncoding> source = nullptr;

        status = impl->utils->CreateBlob(data, dataSize, DXC_CP_UTF8, &source);

        if (!SUCCEEDED(status))
        {
            this->errorMessage = "Failed to load source file";
            return ERR_FailedToLoadFile;
        }

        DxcBuffer buffer;
        buffer.Ptr = source->GetBufferPointer();
        buffer.Size = source->GetBufferSize();
        buffer.Encoding = DXC_CP_UTF8;

        return CompileSpirv(buffer, config);
    }

    ShaderCompiler::ErrorCode ShaderCompiler::Compile(ShaderBlobFormat outFormat, const IO::Path& hlslPath,
                                                      ShaderCompilationInfo& config)
    {
		if (outFormat == ShaderBlobFormat::MSL)
        {
            return CompileMSL(hlslPath, config);
        }
        
        return CompileSpirv(hlslPath, config);
    }

    ShaderCompiler::ErrorCode ShaderCompiler::Compile(ShaderBlobFormat outFormat, const void* data, u32 dataSize, ShaderCompilationInfo& config)
    {
		if (outFormat == ShaderBlobFormat::MSL)
        {
            return CompileMSL(data, dataSize, config);
        }

        return CompileSpirv(data, dataSize, config);
    }

    ShaderCompiler::ErrorCode ShaderCompiler::CompileAuto(const IO::Path& hlslPath, ShaderCompilationInfo& config)
    {
		ShaderBlobFormat outFormat = ShaderBlobFormat::Spirv;
        if (gDynamicRHI != nullptr)
        {
	        switch (gDynamicRHI->GetGraphicsBackend())
	        {
	        case GraphicsBackend::Metal:
				outFormat = ShaderBlobFormat::MSL;
		        break;
	        default:
            case GraphicsBackend::Vulkan:
				outFormat = ShaderBlobFormat::Spirv;
                break;
	        }
        }
		return Compile(outFormat, hlslPath, config);
    }

    ShaderCompiler::ErrorCode ShaderCompiler::CompileAuto(const void* data, u32 dataSize, ShaderCompilationInfo& config)
    {
		ShaderBlobFormat outFormat = ShaderBlobFormat::Spirv;
        if (gDynamicRHI != nullptr)
        {
            switch (gDynamicRHI->GetGraphicsBackend())
            {
            case GraphicsBackend::Metal:
                outFormat = ShaderBlobFormat::MSL;
                break;
            default:
            case GraphicsBackend::Vulkan:
                outFormat = ShaderBlobFormat::Spirv;
                break;
            }
        }
		return Compile(outFormat, data, dataSize, config);
    }

    ShaderCompiler::ErrorCode ShaderCompiler::BuildSpirv(const IO::Path& hlslPath, const ShaderBuildConfig& buildConfig, BinaryBlob& outByteCode, Array<std::wstring>& extraArgs)
	{
		if (!hlslPath.Exists())
			return ERR_FileNotFound;
		if (hlslPath.IsDirectory())
			return ERR_InvalidFile;

		HRESULT status;
		ShaderBuildConfig& config = const_cast<ShaderBuildConfig&>(buildConfig);

		CComPtr<IDxcBlobEncoding> source = nullptr;
		std::wstring str = ToWString(hlslPath.GetString());
		std::wstring fileNameStr = ToWString(hlslPath.GetFileName().GetString());

		status = impl->utils->LoadFile(str.data(), nullptr, &source);
		
		if (!SUCCEEDED(status))
		{
			this->errorMessage = "Failed to load source file";
			return ERR_FailedToLoadFile;
		}

		DxcBuffer buffer;
		buffer.Ptr = source->GetBufferPointer();
		buffer.Size = source->GetBufferSize();
		buffer.Encoding = DXC_CP_UTF8;

		return BuildSpirv(buffer, config, outByteCode, extraArgs);
	}

	ShaderCompiler::ErrorCode ShaderCompiler::BuildSpirv(const void* data, u32 dataSize, const ShaderBuildConfig& buildConfig, 
		BinaryBlob& outByteCode, Array<std::wstring>& extraArgs)
	{
#if SHADER_COMPILATION_SUPPORTED
		HRESULT status = 0;
		ShaderBuildConfig& config = const_cast<ShaderBuildConfig&>(buildConfig);

		CComPtr<IDxcBlobEncoding> source = nullptr;

		status = impl->utils->CreateBlob(data, dataSize, DXC_CP_UTF8, &source);

		if (!SUCCEEDED(status))
		{
			this->errorMessage = "Failed to load source file";
			return ERR_FailedToLoadFile;
		}

		DxcBuffer buffer;
		buffer.Ptr = source->GetBufferPointer();
		buffer.Size = source->GetBufferSize();
		buffer.Encoding = DXC_CP_UTF8;

		return BuildSpirv(buffer, config, outByteCode, extraArgs);
#else
		return ERR_UnsupportedPlatform;
#endif
	}

    ShaderCompiler::ErrorCode ShaderCompiler::CompileMSL(DxcBuffer buffer, ShaderCompilationInfo& config)
    {
#if SHADER_COMPILATION_SUPPORTED
        HRESULT status = 0;
        
        if (config.outReflection == nullptr)
        {
            return ERR_ReflectionRequired;
        }
        
        Array<std::wstring> globalDefinesWString = config.globalDefines.Transform<std::wstring>([&](String& string)
            {
                return ToWString(string);
            });
        
        RHI::ShaderStage stageMasks = RHI::ShaderStage::None;
        
        for (int i = 0; i < config.stages.GetSize(); i++)
        {
            auto& stageInfo = config.stages[i];
            
            Array<const wchar_t*> wcharArgs{};
            for (const auto& arg : stageInfo.extraArgs)
            {
                wcharArgs.Add(arg.data());
            }
            
            wcharArgs.Add(L"-spirv");
            
            std::wstring entryName = ToWString(stageInfo.entryPoint);
            
            wcharArgs.AddRange({ L"-E", entryName.data() });
            
            if (stageInfo.stage == RHI::ShaderStage::Vertex)
            {
                wcharArgs.AddRange({ L"-T", L"vs_6_0" });
            }
            else if (stageInfo.stage == RHI::ShaderStage::Fragment)
            {
                wcharArgs.AddRange({ L"-T", L"ps_6_0" });
            }
            else if (stageInfo.stage == RHI::ShaderStage::Compute)
            {
                wcharArgs.AddRange({ L"-T", L"cs_6_0" });
            }
            else
            {
                continue;
            }
            
            for (const auto& define : globalDefinesWString)
            {
                wcharArgs.AddRange({ L"-D", define.data() });
            }
            
            Array<std::wstring> stageDefinesWString = stageInfo.stageDefines.Transform<std::wstring>([&](String& string)
                {
                    return ToWString(string);
                });
            
            for (const auto& define : stageDefinesWString)
            {
                wcharArgs.AddRange({ L"-D", define.data() });
            }
            
            Array<std::wstring> includeSearchPathsWString = config.includeSearchPaths.Transform<std::wstring>([&](IO::Path& path) -> std::wstring
                {
                    return ToWString(path.GetString());
                });
            
            for (const auto& includePath : includeSearchPathsWString)
            {
                wcharArgs.AddRange({ L"-I", includePath.data() });
            }
            
            CComPtr<IDxcResult> results;
            status = impl->compiler->Compile(
                &buffer,                // Source buffer.
                wcharArgs.GetData(),     // Array of pointers to arguments.
                (u32)wcharArgs.GetSize(),      // Number of arguments.
                impl->includeHandler,    // User-provided interface to handle #include directives (optional).
                IID_PPV_ARGS(&results)  // Compiler output status, buffer, and errors.
            );
            
            defer(&)
            {
                results.Release();
            };
            
            results->GetStatus(&status);

            if (!SUCCEEDED(status))
            {
                CComPtr<IDxcBlobEncoding> error;
                results->GetErrorBuffer(&error);
                char* message = (char*)error->GetBufferPointer();
                auto size = error->GetBufferSize();
                this->errorMessage = message;
                return ERR_CompilationFailure;
            }

            CComPtr<IDxcBlob> blob;
            status = results->GetResult(&blob);

            if (!SUCCEEDED(status))
            {
                return ERR_CompilationFailure;
            }
            
            if (config.outReflection != nullptr)
            {
                ShaderReflector reflector{};
                auto reflectionError = reflector.ReflectSpirv(blob->GetBufferPointer(), (u32)blob->GetBufferSize(), stageInfo.stage, *config.outReflection, stageInfo.entryPoint);
                
                if (reflectionError != ShaderReflector::ERR_Success)
                {
                    return ERR_ReflectionFailure;
                }
            }
            
            stageInfo.outByteCode->LoadData(blob->GetBufferPointer(), blob->GetBufferSize());
            
            stageMasks |= stageInfo.stage;
        }
        
        for (auto& stageInfo : config.stages)
        {
            RHI::ShaderStage curStage = stageInfo.stage;
            
            BinaryBlob spirvByteCode = MoveTemp(*stageInfo.outByteCode);
            
            spirv_cross::CompilerMSL compiler((u32*)spirvByteCode.GetDataPtr(), spirvByteCode.GetDataSize() / 4);
            
            spirv_cross::CompilerMSL::Options mslOptions;
            mslOptions.platform = spirv_cross::CompilerMSL::Options::Platform::macOS;
            mslOptions.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(2, 3);
            mslOptions.argument_buffers = true;
            mslOptions.force_active_argument_buffer_resources = true;
            mslOptions.pad_argument_buffer_resources = true;
            compiler.set_msl_options(mslOptions);
            
            for (int i = 0; i < config.outReflection->srgLayouts.GetSize(); i++)
            {
                for (int j = 0; j < config.outReflection->srgLayouts[i].variables.GetSize(); j++)
                {
                    const auto& variable = config.outReflection->srgLayouts[i].variables[j];
                    
                    spirv_cross::MSLResourceBinding mslBinding{};
                    mslBinding.desc_set = (uint32_t)config.outReflection->srgLayouts[i].srgType;
                    mslBinding.binding = variable.bindingSlot;
                    mslBinding.stage = ShaderStageToExecutionModel(curStage);
                    mslBinding.count = 1;
                    
                    switch (variable.type)
                    {
                        case RHI::ShaderResourceType::None:
                            continue;
                        case RHI::ShaderResourceType::ConstantBuffer:
                        case RHI::ShaderResourceType::StructuredBuffer:
                        case RHI::ShaderResourceType::RWStructuredBuffer:
                            mslBinding.basetype = spirv_cross::SPIRType::Void;
                            mslBinding.msl_buffer = variable.bindingSlot;
                            break;
                        case RHI::ShaderResourceType::Texture1D:
                        case RHI::ShaderResourceType::Texture2D:
                        case RHI::ShaderResourceType::Texture2DArray:
                        case RHI::ShaderResourceType::Texture3D:
                        case RHI::ShaderResourceType::TextureCube:
                        case RHI::ShaderResourceType::RWTexture2D:
                        case RHI::ShaderResourceType::RWTexture3D:
                        case RHI::ShaderResourceType::RWTexture2DArray:
                        case RHI::ShaderResourceType::SubpassInput:
                            mslBinding.basetype = spirv_cross::SPIRType::Image;
                            mslBinding.msl_texture = variable.bindingSlot;
                            break;
                        case RHI::ShaderResourceType::SamplerState:
                            mslBinding.basetype = spirv_cross::SPIRType::Sampler;
                            mslBinding.msl_sampler = variable.bindingSlot;
                            break;
                    }
                    
                    auto add_msl_binding = [&]
                    {
                        try
                        {
                            compiler.add_msl_resource_binding(mslBinding);
                        }
                        catch (const std::exception& e)
                        {
                            const char* msg = e.what();
                            String::IsAlphabet('a');
                        }
                    };
                    
                    add_msl_binding();
                }
            }
            
            try
            {
                std::string resultStr = compiler.compile();
                if (resultStr.empty())
                    return ERR_CompilationFailure;
                
                stageInfo.outByteCode->LoadData(resultStr.c_str(), resultStr.length() + 1); // +1 for null-terminator
            }
            catch (const std::exception& e)
            {
                const char* msg = e.what();
                
                return ERR_CompilationFailure;
            }
        }
        
        return ERR_Success;
#else
		return ERR_UnsupportedPlatform;
#endif
    }

    ShaderCompiler::ErrorCode ShaderCompiler::CompileSpirv(DxcBuffer buffer, ShaderCompilationInfo& config)
    {
        HRESULT status = 0;

        if (config.outReflection == nullptr)
        {
            return ERR_ReflectionRequired;
        }

        Array<std::wstring> globalDefinesWString = config.globalDefines.Transform<std::wstring>([&](String& string)
            {
                return ToWString(string);
            });

        RHI::ShaderStage stageMasks = RHI::ShaderStage::None;

        for (int i = 0; i < config.stages.GetSize(); i++)
        {
            auto& stageInfo = config.stages[i];

            Array<const wchar_t*> wcharArgs{};
            for (const auto& arg : stageInfo.extraArgs)
            {
                wcharArgs.Add(arg.data());
            }

            wcharArgs.Add(L"-spirv");

            std::wstring entryName = ToWString(stageInfo.entryPoint);

            wcharArgs.AddRange({ L"-E", entryName.data() });

            if (stageInfo.stage == RHI::ShaderStage::Vertex)
            {
                wcharArgs.AddRange({ L"-T", L"vs_6_0" });
            }
            else if (stageInfo.stage == RHI::ShaderStage::Fragment)
            {
                wcharArgs.AddRange({ L"-T", L"ps_6_0" });
            }
            else if (stageInfo.stage == RHI::ShaderStage::Compute)
            {
                wcharArgs.AddRange({ L"-T", L"cs_6_0" });
            }
            else
            {
                continue;
            }

            for (const auto& define : globalDefinesWString)
            {
                wcharArgs.AddRange({ L"-D", define.data() });
            }

            Array<std::wstring> stageDefinesWString = stageInfo.stageDefines.Transform<std::wstring>([&](String& string)
                {
                    return ToWString(string);
                });

            for (const auto& define : stageDefinesWString)
            {
                wcharArgs.AddRange({ L"-D", define.data() });
            }

            Array<std::wstring> includeSearchPathsWString = config.includeSearchPaths.Transform<std::wstring>([&](IO::Path& path) -> std::wstring
                {
                    return ToWString(path.GetString());
                });

            for (const auto& includePath : includeSearchPathsWString)
            {
                wcharArgs.AddRange({ L"-I", includePath.data() });
            }

            CComPtr<IDxcResult> results;
            status = impl->compiler->Compile(
                &buffer,                // Source buffer.
                wcharArgs.GetData(),     // Array of pointers to arguments.
                (u32)wcharArgs.GetSize(),      // Number of arguments.
                impl->includeHandler,    // User-provided interface to handle #include directives (optional).
                IID_PPV_ARGS(&results)  // Compiler output status, buffer, and errors.
            );

            defer(&)
            {
                results.Release();
            };

            results->GetStatus(&status);

            if (!SUCCEEDED(status))
            {
                CComPtr<IDxcBlobEncoding> error;
                results->GetErrorBuffer(&error);
                char* message = (char*)error->GetBufferPointer();
                auto size = error->GetBufferSize();
                this->errorMessage = message;
                return ERR_CompilationFailure;
            }

            CComPtr<IDxcBlob> blob;
            status = results->GetResult(&blob);

            if (!SUCCEEDED(status))
            {
                return ERR_CompilationFailure;
            }

            if (config.outReflection != nullptr)
            {
                ShaderReflector reflector{};
                auto reflectionError = reflector.ReflectSpirv(blob->GetBufferPointer(), (u32)blob->GetBufferSize(), stageInfo.stage, *config.outReflection, stageInfo.entryPoint);

                if (reflectionError != ShaderReflector::ERR_Success)
                {
                    return ERR_ReflectionFailure;
                }
            }

            stageInfo.outByteCode->LoadData(blob->GetBufferPointer(), blob->GetBufferSize());

            stageMasks |= stageInfo.stage;
        }

        return ERR_Success;
    }

    ShaderCompiler::ErrorCode ShaderCompiler::Compile(ShaderBlobFormat outFormat, DxcBuffer buffer,
                                                      ShaderCompilationInfo& config)
    {
		if (outFormat == ShaderBlobFormat::MSL)
        {
            return CompileMSL(buffer, config);
        }
        else if (outFormat == ShaderBlobFormat::Spirv)
        {
            return CompileSpirv(buffer, config);
        }

        return ERR_InvalidArgs;
    }

    ShaderCompiler::ErrorCode ShaderCompiler::BuildSpirv(DxcBuffer buffer, const ShaderBuildConfig& buildConfig, BinaryBlob& outByteCode, Array<std::wstring>& extraArgs)
	{
		HRESULT status = 0;
		ShaderBuildConfig& config = const_cast<ShaderBuildConfig&>(buildConfig);

		Array<const wchar_t*> wcharArgs{};
		for (const auto& arg : extraArgs)
		{
			wcharArgs.Add(arg.data());
		}

		wcharArgs.Add(L"-spirv");

		Array<std::wstring> includeSearchPathsWString = config.includeSearchPaths.Transform<std::wstring>([&](IO::Path& path) -> std::wstring
			{
				return ToWString(path.GetString());
			});

		for (const auto& includePath : includeSearchPathsWString)
		{
			wcharArgs.AddRange({ L"-I", includePath.data() });
		}

		std::wstring entryName = ToWString(config.entry);

		wcharArgs.AddRange({ L"-E", entryName.data() });

		if (buildConfig.stage == RHI::ShaderStage::Vertex)
		{
			wcharArgs.AddRange({ L"-T", L"vs_6_0" });
		}
		else if (buildConfig.stage == RHI::ShaderStage::Fragment)
		{
			wcharArgs.AddRange({ L"-T", L"ps_6_0" });
		}
		else if (buildConfig.stage == RHI::ShaderStage::Compute)
		{
			wcharArgs.AddRange({ L"-T", L"cs_6_0" });
		}
		else
		{
			return ERR_InvalidArgs;
		}

		Array<std::wstring> globalDefinesWString = config.globalDefines.Transform<std::wstring>([&](String& string)
			{
				return ToWString(string);
			});

		for (const auto& define : globalDefinesWString)
		{
			wcharArgs.AddRange({ L"-D", define.data() });
		}

		CComPtr<IDxcResult> results;
		status = impl->compiler->Compile(
			&buffer,                // Source buffer.
			wcharArgs.GetData(),     // Array of pointers to arguments.
			wcharArgs.GetSize(),      // Number of arguments.
			impl->includeHandler,    // User-provided interface to handle #include directives (optional).
			IID_PPV_ARGS(&results)  // Compiler output status, buffer, and errors.
		);
		
		defer(&)
		{
			results.Release();
		};

		results->GetStatus(&status);

		if (!SUCCEEDED(status))
		{
			CComPtr<IDxcBlobEncoding> error;
			results->GetErrorBuffer(&error);
			char* message = (char*)error->GetBufferPointer();
			auto size = error->GetBufferSize();
			this->errorMessage = message;
			return ERR_CompilationFailure;
		}

		CComPtr<IDxcBlob> blob;
		status = results->GetResult(&blob);

		if (!SUCCEEDED(status))
		{
			return ERR_CompilationFailure;
		}

		outByteCode.LoadData(blob->GetBufferPointer(), blob->GetBufferSize());

		return ERR_Success;
    }


} // namespace CE::Editor

