#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API MaterialAssetDefinition : public AssetDefinition
    {
        CE_CLASS(MaterialAssetDefinition, AssetDefinition)
    protected:

        MaterialAssetDefinition();
        
    public:

        virtual ~MaterialAssetDefinition();

        /// Return an array of extensions of source files that produce this asset type.
        virtual const Array<String>& GetSourceExtensions() override;

        virtual SubClassType<AssetImporter> GetAssetImporterClass() override;

        virtual SubClassType<Asset> GetAssetClass() override;

        SubClass<AssetThumbnailGen> GetThumbnailGeneratorClass() override;

        Color GetColorTag() override;

    };
    
} // namespace CE

#include "MaterialAssetDefinition.rtti.h"
