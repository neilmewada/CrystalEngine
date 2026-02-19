#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FStackBox : public FWidget
    {
        CE_CLASS(FStackBox, FWidget)
    protected:

        FStackBox();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FStackBox.rtti.h"
