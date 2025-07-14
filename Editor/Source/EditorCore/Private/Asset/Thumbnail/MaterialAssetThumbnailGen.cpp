#include "EditorCore.h"

namespace CE::Editor
{

    MaterialAssetThumbnailGen::MaterialAssetThumbnailGen()
    {

    }

    MaterialAssetThumbnailGen::~MaterialAssetThumbnailGen()
    {
        
    }

    bool MaterialAssetThumbnailGen::StartProcessing()
    {
        AssetManager* assetManager = gEngine->GetAssetManager();

        for (const auto& assetPath : assetPaths)
        {
            Ref<CE::MaterialInterface> material = assetManager->LoadAssetAtPath<CE::MaterialInterface>(assetPath);
            materialAssets.Add(material);
        }

        return true;
    }
} // namespace CE

