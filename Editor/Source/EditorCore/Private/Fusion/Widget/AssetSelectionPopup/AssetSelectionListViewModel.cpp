#include "EditorCore.h"

namespace CE
{

    AssetSelectionListViewModel::AssetSelectionListViewModel()
    {

    }

    AssetSelectionListViewModel::~AssetSelectionListViewModel()
    {
        
    }

    void AssetSelectionListViewModel::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

        if (registered)
        {
            registered = false;
            AssetRegistry::Get()->RemoveRegistryListener(this);
        }
    }

    void AssetSelectionListViewModel::OnListViewAssigned()
    {
        Super::OnListViewAssigned();

        if (!registered)
        {
            registered = true;
            AssetRegistry::Get()->AddRegistryListener(this);
        }
    }

    void AssetSelectionListViewModel::UpdateDataList()
    {
        AssetRegistry* registry = AssetRegistry::Get();
        assetDataList = registry->GetAllAssetsOfType(assetClass->GetTypeId());

        if (Ref<FListView> listView = this->listView.Lock())
        {
            listView->OnModelUpdate();
        }
    }

    void AssetSelectionListViewModel::OnAssetPathTreeUpdated(PathTree& pathTree)
    {
        UpdateDataList();
    }

    int AssetSelectionListViewModel::GetRowCount()
    {
        return assetDataList.GetSize();
    }

    void AssetSelectionListViewModel::SetData(int row, FListViewRow& widget)
    {
        FVerticalStack& container = widget.GetChild()->As<FVerticalStack>();

        FLabel& title = container.GetChild(0)->As<FLabel>();
        title.Text(assetDataList[row]->bundleName.GetString());

        FLabel& description = container.GetChild(1)->As<FLabel>();
        description.Text(assetDataList[row]->bundlePath.GetString());
    }

    void AssetSelectionListViewModel::SetAssetClass(ClassType* assetClass)
    {
        this->assetClass = assetClass;

        UpdateDataList();
    }

    AssetData* AssetSelectionListViewModel::GetRowData(int row)
    {
        if (row < 0 || row >= assetDataList.GetSize())
            return nullptr;

        return assetDataList[row];
    }

    void AssetSelectionListViewModel::SetFilter(const String& filter)
    {
        this->filter = filter;
        if (filter.IsEmpty())
        {
            UpdateDataList();
            return;
        }

        String filterLower = filter.ToLower();

        assetDataList.Clear();

        AssetRegistry* registry = AssetRegistry::Get();
        const auto& originalAssetDataList = registry->GetAllAssetsOfType(assetClass->GetTypeId());

        for (AssetData* assetData : originalAssetDataList)
        {
            if (assetData->bundleName.GetString().ToLower().Contains(filterLower))
            {
                assetDataList.Add(assetData);
            }
        }

        if (Ref<FListView> listView = this->listView.Lock())
        {
            listView->OnModelUpdate();
        }
    }

    void AssetSelectionListViewModel::ClearFilter()
    {
        SetFilter("");
    }
} // namespace CE

