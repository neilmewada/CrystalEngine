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

    Color StaticMeshAssetDefinition::GetColorTag()
    {
        return Color(0.2f, 0.8f, 1.0f, 1.0f);
    }

} // namespace CE::Editor
