#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FGridViewItem : public FStyledWidget
    {
        CE_CLASS(FGridViewItem, FStyledWidget)
    protected:

        FGridViewItem();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FGridViewItem.rtti.h"
