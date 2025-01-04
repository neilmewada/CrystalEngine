#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowser::AssetBrowser()
    {

    }

    void AssetBrowser::Construct()
    {
        Super::Construct();

        (*this)
        .Title("Assets")
        .Content(
            FNew(FSplitBox)
            .SplitterBackground(Color::RGBA(26, 26, 26))
            .SplitterHoverBackground(Color::RGBA(128, 128, 128))
            .SplitterSize(5.0f)
            .SplitterDrawRatio(0.5f)
            .Direction(FSplitDirection::Horizontal)
            (
                FNew(FVerticalStack)
                .ContentHAlign(HAlign::Fill)
                .FillRatio(0.2f)
                (
                    FNew(AssetBrowserTreeView)
                    .FillRatio(1.0f)
                ),
                
                FNew(FVerticalStack)
                .ContentHAlign(HAlign::Fill)
                .FillRatio(0.8f)
            )
        )
        .Style("EditorMinorDockTab");
    }
    
}

