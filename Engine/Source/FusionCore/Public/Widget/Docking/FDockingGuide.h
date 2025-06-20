#pragma once

namespace CE
{

    CLASS()
    class FUSIONCORE_API FDockingGuide : public FStyledWidget
    {
        CE_CLASS(FDockingGuide, FStyledWidget)
    protected:

        FDockingGuide();

        void Construct() override;

    public: // - Public API -

        Ref<FDockspace> GetDockspace() { return ownerDockspace.Lock(); }

        void OnDockingHintActivated(Ref<FDockingHint> dockingHint);
        void OnDockingHintDeactivated(Ref<FDockingHint> dockingHint);

    protected: // - Internal -

        WeakRef<FDockspace> ownerDockspace;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FDockspace;
        friend class FDockTabItem;
        friend class FDockingHint;
    };
    
}

#include "FDockingGuide.rtti.h"
