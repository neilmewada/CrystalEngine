#pragma once

namespace CE::Editor
{
    class AssetBrowserTreeView;
    class AssetBrowserTreeViewModel;
    class AssetBrowserGridView;
    class AssetBrowserGridViewModel;


    CLASS(Prefs = Editor)
    class CRYSTALEDITOR_API AssetBrowser : public EditorMinorDockTab, public IAssetRegistryListener
    {
        CE_CLASS(AssetBrowser, EditorMinorDockTab)
    protected:

        AssetBrowser();

        void Construct() override;

        void OnBeginDestroy() override;

        void OnAssetPathTreeUpdated(PathTree& pathTree) override;

    public: // - Public API -

        void SetCurrentPath(const CE::Name& path);

        void OpenAsset(const CE::Name& path);

        bool IsCurrentDirectoryReadOnly() const;

        Array<CE::Name> GetSelectedAssetPaths();

        void BrowseToAsset(const CE::Name& path);

    protected: // - Internal -

        FUNCTION()
        void OnDirectorySelectionChanged(FItemSelectionModel* selectionModel);

        FUNCTION()
        void OnLeftExpansionChanged(FExpandableSection* section);

        FUNCTION()
        void OnAddButtonClicked();

        FUNCTION()
        void OnImportButtonClicked();

        void UpdateBreadCrumbs();

        void UpdateAssetGridView();

        void CreateNewEmptyDirectory();

        bool CreateNewAsset(SubClass<Asset> assetType);

        template<typename TAsset> requires TIsBaseClassOf<Asset, TAsset>::Value
        bool CreateNewAsset()
        {
            return CreateNewAsset(TAsset::StaticClass());
        }

        bool CanRenameDirectory(const CE::Name& originalPath, const CE::Name& newName);
        bool RenameDirectory(const CE::Name& originalPath, const CE::Name& newName);

        bool RenameAsset(const CE::Name& originalPath, const CE::Name& newName);

        void DeleteDirectoriesAndAssets(const Array<CE::Name>& itemPaths);

        Ref<AssetBrowserTreeViewModel> treeViewModel = nullptr;
        Ref<AssetBrowserTreeView> treeView = nullptr;

        Ref<FExpandableSection> directorySection;
        Array<Ref<FExpandableSection>> leftSections;

        Ref<FHorizontalStack> searchBarStack;
        Ref<FButton> addButton;

        Ref<AssetBrowserGridViewModel> gridViewModel = nullptr;
        Ref<AssetBrowserGridView> gridView;
        Ref<FScrollBox> gridViewScrollBox;

        PathTreeNode* currentDirectory = nullptr;
        CE::Name currentPath = {};

        Ref<FHorizontalStack> breadCrumbsContainer = nullptr;

        Array<Ref<FSelectableButton>> selectables;

        FIELD(Prefs)
        CE::Name defaultAssetImportPath;

        FUSION_WIDGET;
        friend class AssetBrowserGridView;
        friend class AssetBrowserItem;
    };

}

#include "AssetBrowser.rtti.h"
