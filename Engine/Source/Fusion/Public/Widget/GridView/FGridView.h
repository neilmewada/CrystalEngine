#pragma once

namespace CE
{
    class FGridViewItem;
    class FGridViewContainer;

    CLASS()
    class FUSION_API FGridView : public FStyledWidget
    {
        CE_CLASS(FGridView, FStyledWidget)
    protected:

        FGridView();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        Ref<FScrollBox> scrollBox;
        Ref<FGridViewContainer> container;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FGridView.rtti.h"
