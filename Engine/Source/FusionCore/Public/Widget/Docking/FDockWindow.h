#pragma once

namespace CE
{
    
    CLASS()
    class FUSIONCORE_API FDockWindow : public FWindow
    {
        CE_CLASS(FDockWindow, FWindow)
    protected:

        FDockWindow();

		virtual ~FDockWindow();

        void Construct() override;

        void OnBeginDestroy() override;

    public: // - Public API -

		Ref<FDockspaceSplitView> GetDockspaceSplitView() { return ownerDockspaceSplitView.Lock(); }

		Ref<FDockspace> GetDockspace() { return ownerDockspace.Lock(); }

    protected: // - Internal -

        void OnFusionPropertyModified(const CE::Name& propertyName) override;

        WeakRef<FDockspace> ownerDockspace;
		WeakRef<FDockspaceSplitView> ownerDockspaceSplitView;
        Ref<FDockTabItem> item;

    public: // - Fusion Properties -

        FUSION_PROPERTY(String, Title);

        FUSION_PROPERTY(bool, CanBeUndocked);
        FUSION_PROPERTY(FDockspaceFilter, AllowedDockspaces);

        FUSION_WIDGET;
        friend class FDockTabWell;
        friend class FDockspaceSplitView;
    };
    
}

#include "FDockWindow.rtti.h"
