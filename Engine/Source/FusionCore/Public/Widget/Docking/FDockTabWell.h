#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockTabWell : public FStyledWidget
    {
        CE_CLASS(FDockTabWell, FStyledWidget)
    protected:

        FDockTabWell();

        void Construct() override;

    public: // - Public API -

        void UpdateTabWell();

    protected: // - Internal -

        WeakRef<FDockspace> owner;

        Ref<FHorizontalStack> container;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FDockspace;
    };
    
}

#include "FDockTabWell.rtti.h"
