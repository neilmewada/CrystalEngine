#pragma once

namespace CE
{
    CLASS()
    class EDITORCORE_API AssetSelectionListViewModel : public FListViewModel, IAssetRegistryListener
    {
        CE_CLASS(AssetSelectionListViewModel, FListViewModel)
    protected:

        AssetSelectionListViewModel();

        virtual ~AssetSelectionListViewModel();

        void OnBeginDestroy() override;

        void OnListViewAssigned() override;

        void UpdateDataList();

    public:

        void OnDirectoryRenamed(const Name& oldPath, const Name& newPath) override;

        void OnAssetPathTreeUpdated(PathTree& pathTree) override;

        int GetRowCount() override;

        void SetData(int row, FListViewRow& widget) override;

        void SetAssetClass(ClassType* assetClass);

        AssetData* GetRowData(int row);

        void SetFilter(const String& filter);

        void ClearFilter();

    private:

        String filter;
        Array<AssetData*> assetDataList;

        bool registered = false;
        ClassType* assetClass = nullptr;
    };
    
} // namespace CE

#include "AssetSelectionListViewModel.rtti.h"
