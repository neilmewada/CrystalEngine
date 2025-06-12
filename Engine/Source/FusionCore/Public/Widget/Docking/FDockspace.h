#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockspace : public FCompoundWidget
    {
        CE_CLASS(FDockspace, FCompoundWidget)
    protected:

        FDockspace();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FDockspace.rtti.h"
