#include "EditorCore.h"

namespace CE::Editor
{

    PhysicsMaterialAssetDefinition::PhysicsMaterialAssetDefinition()
    {

    }

    const Array<String>& PhysicsMaterialAssetDefinition::GetSourceExtensions()
    {
        thread_local Array<String> empty{};
        return empty;
    }

    SubClassType<AssetImporter> PhysicsMaterialAssetDefinition::GetAssetImporterClass()
    {
        return nullptr;
    }

    SubClassType<Asset> PhysicsMaterialAssetDefinition::GetAssetClass()
    {
        return PhysicsMaterial::StaticClass();
    }

    CE::Name PhysicsMaterialAssetDefinition::GetIconPath()
    {
        return "/Editor/Assets/Icons/AssetIcons/PhysicsMaterial";
    }

    Color PhysicsMaterialAssetDefinition::GetColorTag()
    {
        return Color(0.2f, 1.0f, 0.3f, 1.0f);
    }

} // namespace CE

