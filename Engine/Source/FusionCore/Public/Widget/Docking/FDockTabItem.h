#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockTabItem : public FStyledWidget
    {
        CE_CLASS(FDockTabItem, FStyledWidget)
    protected:

        FDockTabItem();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FDockTabItem.rtti.h"
