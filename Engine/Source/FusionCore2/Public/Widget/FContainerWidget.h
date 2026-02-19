#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FContainerWidget : public FWidget
    {
        CE_CLASS(FContainerWidget, FWidget)
    protected:

        FContainerWidget();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FContainerWidget.rtti.h"
