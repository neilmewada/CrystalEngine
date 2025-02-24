#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FGridViewItem : public FWidget
    {
        CE_CLASS(FGridViewItem, FWidget)
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
