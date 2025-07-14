#include "EditorCore.h"

namespace CE::Editor
{

    MaterialAssetDefinition::MaterialAssetDefinition()
    {

    }

    MaterialAssetDefinition::~MaterialAssetDefinition()
    {
        
    }

    const Array<String>& MaterialAssetDefinition::GetSourceExtensions()
    {
        thread_local Array<String> empty;
        return empty;
    }

    SubClassType<AssetImporter> MaterialAssetDefinition::GetAssetImporterClass()
    {
        return nullptr;
    }

    SubClassType<Asset> MaterialAssetDefinition::GetAssetClass()
    {
        return CE::MaterialInterface::StaticClass();
    }

    SubClass<AssetThumbnailGen> MaterialAssetDefinition::GetThumbnailGeneratorClass()
    {
        return MaterialAssetThumbnailGen::StaticClass();
    }
} // namespace CE

