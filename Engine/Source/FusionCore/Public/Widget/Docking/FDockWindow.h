#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockWindow : public FWindow
    {
        CE_CLASS(FDockWindow, FWindow)
    protected:

        FDockWindow();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties -
        

        FUSION_WIDGET;
    };
    
}

#include "FDockWindow.rtti.h"
