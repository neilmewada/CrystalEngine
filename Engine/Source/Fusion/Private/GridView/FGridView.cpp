#include "Fusion.h"

namespace CE
{

    FGridView::FGridView()
    {

    }

    void FGridView::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FScrollBox, scrollBox)
            .VerticalScroll(true)
            .HorizontalScroll(false)
            .Child(
                FAssignNew(FGridViewContainer, container)
                .GridView(this)
                .HAlign(HAlign::Left)
                .VAlign(VAlign::Top)
            )
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
        );
    }
    
}

