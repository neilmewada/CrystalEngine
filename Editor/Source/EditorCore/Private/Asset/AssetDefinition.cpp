#include "EditorCore.h"

namespace CE::Editor
{

    AssetDefinition::AssetDefinition()
    {
        
    }

    AssetDefinition::~AssetDefinition()
    {
        
    }

    CE::Name AssetDefinition::GetIconPath()
    {
        return "/Editor/Assets/Icons/AssetIcons/DefaultFile";
    }

    String AssetDefinition::GetTypeDisplayName()
    {
        return GetAssetClass()->GetName().GetLastComponent();
    }

    SubClass<AssetThumbnailGen> AssetDefinition::GetThumbnailGeneratorClass()
    {
        return nullptr;
    }
} // namespace CE::Editor
