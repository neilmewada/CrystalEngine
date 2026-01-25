#pragma once

namespace CE
{
    class FDockspaceSplitView;

    CLASS()
    class FUSIONCORE_API FDockingGuide : public FStyledWidget
    {
        CE_CLASS(FDockingGuide, FStyledWidget)
    protected:

        FDockingGuide();

        void Construct() override;

    public: // - Public API -

        Ref<FDockspaceSplitView> GetDockspaceSplitView() { return ownerDockspaceSplitView.Lock(); }

        void OnDockingHintActivated(Ref<FDockingHint> dockingHint);
        void OnDockingHintDeactivated(Ref<FDockingHint> dockingHint);

    protected: // - Internal -

        WeakRef<FDockspaceSplitView> ownerDockspaceSplitView;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FDockspace;
        friend class FDockTabItem;
        friend class FDockingHint;
        friend class FDockspaceSplitView;
    };
    
}

#include "FDockingGuide.rtti.h"
