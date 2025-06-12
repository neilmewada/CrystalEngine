#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockTabWell : public FWidget
    {
        CE_CLASS(FDockTabWell, FWidget)
    protected:

        FDockTabWell();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FDockTabWell.rtti.h"
