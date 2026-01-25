#include "EditorCore.h"

namespace CE::Editor
{

    SceneAssetDefinition::SceneAssetDefinition()
    {

    }

    const Array<String>& SceneAssetDefinition::GetSourceExtensions()
    {
        thread_local Array<String> empty{};
        return empty;
    }

    SubClassType<AssetImporter> SceneAssetDefinition::GetAssetImporterClass()
    {
        return nullptr;
    }

    SubClassType<Asset> SceneAssetDefinition::GetAssetClass()
    {
        return CE::Scene::StaticClass();
    }

    SubClass<AssetThumbnailGen> SceneAssetDefinition::GetThumbnailGeneratorClass()
    {
        return nullptr;
    }

    CE::Name SceneAssetDefinition::GetIconPath()
    {
	    return "/Editor/Assets/Icons/SceneFile";
    }

    Color SceneAssetDefinition::GetColorTag()
    {
	    return Color(0.95f, 0.9f, 0.2f, 1.0f);
    }

} // namespace CE

