#include "FusionCore.h"

namespace CE
{

    FDockingGuide::FDockingGuide()
    {
        
    }

    void FDockingGuide::Construct()
    {
        Super::Construct();

        constexpr f32 gap = 8;

        Child(
			FNew(FVerticalStack)
            .Gap(gap)
            .ContentHAlign(HAlign::Center)
            .VAlign(VAlign::Center)
            (
                FNew(FDockingHint)
                .HintPosition(FDockingHintPosition::Top),

                FNew(FHorizontalStack)
                .Gap(gap)
                (
                    FNew(FDockingHint)
                    .HintPosition(FDockingHintPosition::Left),

                    FNew(FDockingHint)
                    .HintPosition(FDockingHintPosition::Center),

                    FNew(FDockingHint)
                    .HintPosition(FDockingHintPosition::Right)
                ),

                FNew(FDockingHint)
                .HintPosition(FDockingHintPosition::Bottom)
            )
        );
    }

    void FDockingGuide::OnDockingHintActivated(Ref<FDockingHint> dockingHint)
    {
        if (!dockingHint)
            return;

        
    }

    void FDockingGuide::OnDockingHintDeactivated(Ref<FDockingHint> dockingHint)
    {
        if (!dockingHint)
            return;

    }
}

