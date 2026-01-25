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

    Color AssetDefinition::GetColorTag()
    {
        return Color(0.7f, 0.7f, 0.7f, 1.0f);
    }
} // namespace CE::Editor
