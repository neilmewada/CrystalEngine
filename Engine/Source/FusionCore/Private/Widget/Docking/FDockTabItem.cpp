#include "FusionCore.h"

namespace CE
{

    FDockTabItem::FDockTabItem()
    {

    }

    void FDockTabItem::Construct()
    {
        Super::Construct();

        Child(
            FNew(FHorizontalStack)
            .ContentVAlign(VAlign::Center)
            .ContentHAlign(HAlign::Left)
            .Padding(Vec4(1, 1, 1, 1) * 5)
            (
                FAssignNew(FLabel, tabTitle)
                .FontSize(11)
            )
        );
    }
    
}

