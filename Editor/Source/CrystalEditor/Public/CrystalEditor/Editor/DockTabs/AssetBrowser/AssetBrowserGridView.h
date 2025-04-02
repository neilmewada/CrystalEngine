#pragma once

namespace CE::Editor
{
    class AssetBrowserItem;

    CLASS()
    class CRYSTALEDITOR_API AssetBrowserGridView : public FWrapBox
    {
        CE_CLASS(AssetBrowserGridView, FWrapBox)
    protected:

        AssetBrowserGridView();

        void Construct() override;

        void HandleEvent(FEvent* event) override;

        void OnSelectionUpdated();

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDragEvents() const override { return true; }

        bool SupportsKeyboardFocus() const override { return true; }

        bool SupportsKeyboardEvents() const override { return true; }

        int GetSelectedItemCount();

        // Will redraw the items
        void OnModelUpdate();

        void DeselectAll();

    protected: // - Internal -

        Array<AssetBrowserItem*> items;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(WeakRef<AssetBrowser>, Owner);
        FUSION_PROPERTY(Ref<AssetBrowserGridViewModel>, Model);

        FUSION_WIDGET;
    };
    
}

#include "AssetBrowserGridView.rtti.h"
