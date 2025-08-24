#include "EditorCore.h"

namespace CE::Editor
{

    ComputeShaderAssetImporter::ComputeShaderAssetImporter()
    {

    }

    ComputeShaderAssetImporter::~ComputeShaderAssetImporter()
    {
        
    }

    Array<AssetImportJob*> ComputeShaderAssetImporter::CreateImportJobs(const Array<IO::Path>& sourceAssets,
        const Array<IO::Path>& productAssets)
    {
        Array<AssetImportJob*> jobs{};

        for (int i = 0; i < sourceAssets.GetSize(); i++)
        {
            jobs.Add(new ComputeShaderAssetImportJob(this, sourceAssets[i], productAssets[i]));
        }

        return jobs;
    }

    bool ComputeShaderAssetImportJob::ProcessAsset(const Ref<Bundle>& bundle)
    {
        if (bundle == nullptr)
            return false;

        if (!sourcePath.Exists())
            return false;

        // NOTE: The bundle might already have assets & objects stored in it if asset already existed
        // It is responsibility of the derived asset importer to clear the old objects or just modify them as per need.

        // Clear the bundle of any subobjects, we will build the asset from scratch
        bundle->DestroyAllSubObjects();

        Array<IO::Path> includePaths = this->includePaths;
        includePaths.Add(sourcePath.GetParentPath());

        Ref<CE::ComputeShader> shader = bundle->LoadObject<CE::ComputeShader>();

        if (shader == nullptr) // Create new object from scratch
        {
            shader = CreateObject<CE::ComputeShader>(bundle.Get(), "ComputeShader");
        }

        FileStream fileReader = FileStream(sourcePath, Stream::Permissions::ReadOnly);
        fileReader.SetAsciiMode(true);

        ComputeShaderPreprocessor preprocessor{ &fileReader, includePaths };

        ComputeShaderPreprocessData preprocessData;
        preprocessor.PreprocessShader(&preprocessData);

        if (preprocessor.GetErrorMessage().NotEmpty())
        {
            errorMessage = preprocessor.GetErrorMessage();
            shader->BeginDestroy();
            return false;
        }

        if (preprocessData.kernels.IsEmpty())
        {
            errorMessage = "No kernels found";
            return false;
        }

        shader->kernelNames = preprocessData.kernels;
        bool reflectionDone = false;

        for (const auto& kernel : preprocessData.kernels)
        {
            ShaderBuildConfig buildConfig{};
            buildConfig.entry = kernel.GetString();
            buildConfig.stage = RHI::ShaderStage::Compute;
            buildConfig.includeSearchPaths = includePaths;
            buildConfig.debugName = kernel.GetString();
            buildConfig.shaderModel = HlslShaderModel::SM_6_0;

            Array<std::wstring> extraArgs{};
            extraArgs.AddRange({
                L"-D", L"COMPILE=1",
                L"-D", L"COMPUTE=1",
                L"-fspv-preserve-bindings",
                L"-fspv-debug=vulkan-with-source"
            });

            ShaderCompiler compiler{};

            shader->kernels.Add({});

            ShaderCompiler::ErrorCode result = compiler.BuildSpirv(sourcePath, buildConfig, shader->kernels.Top(), extraArgs);
            if (result != ShaderCompiler::ERR_Success)
            {
                errorMessage = "Failed to compile compute shader. Error: " + compiler.GetErrorMessage();
                return false;
            }

            if (!reflectionDone)
            {
                ShaderReflector reflector{};
                ShaderReflector::ErrorCode result2 = reflector.ReflectSpirv(shader->kernels.Top().GetDataPtr(),
                    shader->kernels.Top().GetDataSize(), ShaderStage::Compute, shader->reflection, kernel.GetString());

                if (result2 != ShaderReflector::ERR_Success)
                {
                    errorMessage = "Failed to reflect compute shader!";
                    return false;
                }

                reflectionDone = true;
            }
        }

        return true;
    }


} // namespace CE

