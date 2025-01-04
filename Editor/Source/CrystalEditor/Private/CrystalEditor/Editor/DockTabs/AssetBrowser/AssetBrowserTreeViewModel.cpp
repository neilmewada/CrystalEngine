#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowserTreeViewModel::AssetBrowserTreeViewModel()
    {

    }

    AssetBrowserTreeViewModel::~AssetBrowserTreeViewModel()
    {
        
    }

    FModelIndex AssetBrowserTreeViewModel::GetIndex(u32 row, u32 column, const FModelIndex& parent)
    {
        if (!parent.IsValid())
        {
            return CreateIndex(row, column, nullptr);
        }
        return {};
    }

    u32 AssetBrowserTreeViewModel::GetRowCount(const FModelIndex& parent)
    {
        if (!parent.IsValid())
        {
            return 1;
        }
        return 0;
    }

    u32 AssetBrowserTreeViewModel::GetColumnCount(const FModelIndex& parent)
    {
        return 1;
    }

    void AssetBrowserTreeViewModel::SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent)
    {
        FTreeViewRow& treeRow = rowWidget.As<FTreeViewRow>();

        FTreeViewCell& cell = *treeRow.GetCell(0);

        cell
        .Text("File")
        .IconEnabled(false)
            ;
    }

} // namespace CE

