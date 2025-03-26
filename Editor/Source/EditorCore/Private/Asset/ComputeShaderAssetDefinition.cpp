#include "EditorCore.h"

namespace CE
{

    ComputeShaderAssetDefinition::ComputeShaderAssetDefinition()
    {

    }

    const Array<String>& ComputeShaderAssetDefinition::GetSourceExtensions()
    {
        static const Array<String> extensions = {
            ".compute"
        };
        return extensions;
    }

    SubClassType<AssetImporter> ComputeShaderAssetDefinition::GetAssetImporterClass()
    {
        return ComputeShaderAssetImporter::StaticClass();
    }

    SubClassType<Asset> ComputeShaderAssetDefinition::GetAssetClass()
    {
        return CE::ComputeShader::StaticClass();
    }
} // namespace CE

