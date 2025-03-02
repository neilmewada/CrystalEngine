#include "EditorCore.h"

namespace CE::Editor
{

    const Array<String>& StaticMeshAssetDefinition::GetSourceExtensions()
    {
        static const Array<String> extensions{
            ".fbx"
        };

        return extensions;
    }

    SubClass<AssetImporter> StaticMeshAssetDefinition::GetAssetImporterClass()
    {
        return GetStaticClass<StaticMeshAssetImporter>();
    }

    SubClassType<Asset> StaticMeshAssetDefinition::GetAssetClass()
    {
        return CE::StaticMesh::StaticClass();
    }
} // namespace CE::Editor
