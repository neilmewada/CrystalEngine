#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowserItem::AssetBrowserItem()
    {
        m_ClipChildren = true;
    }

    void AssetBrowserItem::Construct()
    {
        Super::Construct();

        (*this)
            .Width(80)
            .Height(100)
            .Style("SelectableButton")
            (
                FAssignNew(FLabel, titleLabel)
                .Text("Asset")
                .FontSize(9)
                .Foreground(Color::White())
                .HAlign(HAlign::Center)
                .VAlign(VAlign::Bottom)
            )
        ;
    }
    
}

