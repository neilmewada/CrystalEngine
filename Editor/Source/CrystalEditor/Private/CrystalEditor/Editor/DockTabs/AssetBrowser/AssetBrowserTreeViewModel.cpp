#include "CrystalEditor.h"

namespace CE
{

    AssetBrowserTreeViewModel::AssetBrowserTreeViewModel()
    {

    }

    AssetBrowserTreeViewModel::~AssetBrowserTreeViewModel()
    {
        
    }

    FModelIndex AssetBrowserTreeViewModel::GetIndex(u32 row, u32 column, const FModelIndex& parent)
    {
        return FModelIndex();
    }

    u32 AssetBrowserTreeViewModel::GetRowCount(const FModelIndex& parent)
    {
        return 0;
    }

    u32 AssetBrowserTreeViewModel::GetColumnCount(const FModelIndex& parent)
    {
        return 1;
    }

    void AssetBrowserTreeViewModel::SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent)
    {
    }
} // namespace CE

