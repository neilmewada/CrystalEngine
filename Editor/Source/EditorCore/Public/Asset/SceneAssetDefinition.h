#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API SceneAssetDefinition : public AssetDefinition
    {
        CE_CLASS(SceneAssetDefinition, AssetDefinition)
    protected:

        SceneAssetDefinition();
        
    public:

        /// Return an array of extensions of source files that produce this asset type.
        virtual const Array<String>& GetSourceExtensions() override;

        virtual SubClassType<AssetImporter> GetAssetImporterClass() override;

        virtual SubClassType<Asset> GetAssetClass() override;

        SubClass<AssetThumbnailGen> GetThumbnailGeneratorClass() override;

        CE::Name GetIconPath() override;

        Color GetColorTag() override;

    };
    
} // namespace CE

#include "SceneAssetDefinition.rtti.h"
