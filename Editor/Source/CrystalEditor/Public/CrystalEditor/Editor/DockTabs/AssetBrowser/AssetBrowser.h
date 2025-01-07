#pragma once

namespace CE::Editor
{
    class AssetBrowserTreeView;
    class AssetBrowserTreeViewModel;


    CLASS()
    class CRYSTALEDITOR_API AssetBrowser : public EditorMinorDockTab
    {
        CE_CLASS(AssetBrowser, EditorMinorDockTab)
    protected:

        AssetBrowser();

        void Construct() override;

    public: // - Public API -



    protected: // - Internal -

        FUNCTION()
        void OnDirectorySelectionChanged(FItemSelectionModel* selectionModel);

        Ref<AssetBrowserTreeViewModel> model = nullptr;
        Ref<AssetBrowserTreeView> treeView = nullptr;

        FUSION_WIDGET;
    };
    
}

#include "AssetBrowser.rtti.h"
