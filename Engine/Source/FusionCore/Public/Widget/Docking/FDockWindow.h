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
        Ref<FDockTabItem> item;

    public: // - Fusion Properties -

        FUSION_PROPERTY(String, Title);

        FUSION_WIDGET;
        friend class FDockTabWell;
    };
    
}

#include "FDockWindow.rtti.h"
