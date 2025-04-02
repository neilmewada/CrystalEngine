#include "FusionCore.h"

namespace CE
{

    FTextButton::FTextButton()
    {

    }

    void FTextButton::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FLabel, label)
            .Text("Text Button")
            .VAlign(VAlign::Center)
        );
    }

}

