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

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

        WeakRef<FDockspace> ownerDockspace;

    public: // - Fusion Properties -

        FUSION_PROPERTY(String, Title);

        FUSION_WIDGET;
    };
    
}

#include "FDockWindow.rtti.h"
