#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FGridViewContainer : public FWidget
    {
        CE_CLASS(FGridViewContainer, FWidget)
    protected:

        FGridViewContainer();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

    public: // - Fusion Properties - 

        FUSION_PROPERTY(WeakRef<FGridView>, GridView);

        FUSION_WIDGET;
    };
    
}

#include "FGridViewContainer.rtti.h"
