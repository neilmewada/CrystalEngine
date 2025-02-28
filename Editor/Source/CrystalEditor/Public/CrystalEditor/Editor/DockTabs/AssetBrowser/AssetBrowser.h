#pragma once

namespace CE::Editor
{
    class AssetBrowserTreeView;
    class AssetBrowserTreeViewModel;
    class AssetBrowserGridView;
    class AssetBrowserGridViewModel;


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

        FUNCTION()
        void OnLeftExpansionChanged(FExpandableSection* section);

        void UpdateAssetGridView();

        Ref<AssetBrowserTreeViewModel> treeViewModel = nullptr;
        Ref<AssetBrowserTreeView> treeView = nullptr;

        Ref<FExpandableSection> directorySection;
        Array<Ref<FExpandableSection>> leftSections;

        Ref<FHorizontalStack> searchBarStack;

        Ref<AssetBrowserGridViewModel> gridViewModel = nullptr;
        Ref<AssetBrowserGridView> gridView;

        PathTreeNode* currentDirectory = nullptr;
        CE::Name currentPath = {};

        Array<Ref<FSelectableButton>> selectables;

        FUSION_WIDGET;
    };
    
}

#include "AssetBrowser.rtti.h"
