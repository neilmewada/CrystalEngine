#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API PhysicsMaterialAssetDefinition : public AssetDefinition
    {
        CE_CLASS(PhysicsMaterialAssetDefinition, AssetDefinition)
    protected:

        PhysicsMaterialAssetDefinition();
        
    public:

        /// Return an array of extensions of source files that produce this asset type.
        virtual const Array<String>& GetSourceExtensions() override;

        virtual SubClassType<AssetImporter> GetAssetImporterClass() override;

        virtual SubClassType<Asset> GetAssetClass() override;

        CE::Name GetIconPath() override;

        Color GetColorTag() override;

    };
    
} // namespace CE

#include "PhysicsMaterialAssetDefinition.rtti.h"
