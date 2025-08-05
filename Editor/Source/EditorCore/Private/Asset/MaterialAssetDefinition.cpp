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
        thread_local Array<String> empty{};
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

    Color MaterialAssetDefinition::GetColorTag()
    {
        return Color(1.0f, 0.6f, 0.2f, 1.0f);
    }
} // namespace CE

