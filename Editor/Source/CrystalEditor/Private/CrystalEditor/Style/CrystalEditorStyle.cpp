#include "CrystalEditor.h"

namespace CE
{

    CrystalEditorStyle::CrystalEditorStyle()
    {

    }

    CrystalEditorStyle::~CrystalEditorStyle()
    {

    }

    void CrystalEditorStyle::InitEditorStyle()
    {
        Super::InitEditorStyle();

        if (!assetBrowserItem)
        {
            assetBrowserItem = CreateObject<AssetBrowserItemStyle>(this, "AssetBrowserItem");
            Add(assetBrowserItem.Get());
        }
    }

    
} // namespace CE

