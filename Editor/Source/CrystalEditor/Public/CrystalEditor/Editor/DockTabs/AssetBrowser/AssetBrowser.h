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

        FUNCTION()
        void OnLeftExpansionChanged(FExpandableSection* section);

        void UpdateAssetGridView();

        Ref<AssetBrowserTreeViewModel> model = nullptr;
        Ref<AssetBrowserTreeView> treeView = nullptr;

        Ref<FExpandableSection> directorySection;
        Array<Ref<FExpandableSection>> leftSections;

        Ref<FHorizontalStack> searchBarStack;

        PathTreeNode* currentDirectory = nullptr;

        FUSION_WIDGET;
    };
    
}

#include "AssetBrowser.rtti.h"
