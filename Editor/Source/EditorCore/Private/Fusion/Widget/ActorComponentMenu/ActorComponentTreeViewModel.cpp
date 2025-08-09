#include "EditorCore.h"

namespace CE::Editor
{

    ActorComponentTreeViewModel::ActorComponentTreeViewModel()
    {

    }

    FModelIndex ActorComponentTreeViewModel::GetParent(const FModelIndex& index)
    {
        return {};
    }

    FModelIndex ActorComponentTreeViewModel::GetIndex(u32 row, u32 column, const FModelIndex& parent)
    {
        return {};
    }

    u32 ActorComponentTreeViewModel::GetRowCount(const FModelIndex& parent)
    {
        return {};
    }

    u32 ActorComponentTreeViewModel::GetColumnCount(const FModelIndex& parent)
    {
        return {};
    }

    void ActorComponentTreeViewModel::SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent)
    {
    }
} // namespace CE

