#pragma once

namespace CE
{
    CLASS()
    class EDITORCORE_API ComputeShaderAssetDefinition : public AssetDefinition
    {
        CE_CLASS(ComputeShaderAssetDefinition, AssetDefinition)
    protected:

        ComputeShaderAssetDefinition();
        
    public:

        /// Return an array of extensions of source files that produce this asset type.
        virtual const Array<String>& GetSourceExtensions() override;

        virtual SubClassType<AssetImporter> GetAssetImporterClass() override;

        virtual SubClassType<Asset> GetAssetClass() override;

    };
    
} // namespace CE

#include "ComputeShaderAssetDefinition.rtti.h"
