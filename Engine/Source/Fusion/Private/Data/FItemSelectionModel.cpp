#include "Fusion.h"

namespace CE
{

    FItemSelectionModel::FItemSelectionModel()
    {

    }

    FItemSelectionModel::~FItemSelectionModel()
    {
        
    }

    FModelIndex FItemSelectionModel::GetSelectedIndex()
    {
        if (selectionStack.IsEmpty())
            return {};
        return selectionStack.GetLast();
    }

    void FItemSelectionModel::Select(const FModelIndex& index)
    {
        if (selectionMode == FSelectionMode::Single)
        {
            selection.Clear();
            selectionStack.Clear();
        }
        else if (selectionMode == FSelectionMode::None)
        {
            selection.Clear();
            selectionStack.Clear();
	        return;
        }

        selection.Add(index);
        selectionStack.Add(index);
        
        ValidateSelection();

        m_OnSelectionChanged.Broadcast(this);
    }

    void FItemSelectionModel::Deselect(const FModelIndex& index)
    {
        if (selection.Exists(index))
        {
            selection.Remove(index);
            selectionStack.Remove(index);

            m_OnSelectionChanged.Broadcast(this);
        }
    }

    bool FItemSelectionModel::IsSelected(const FModelIndex& index)
    {
        auto model = index.GetModel();

        if (selectionType == FSelectionType::Row)
        {
            FModelIndex parent = model->GetParent(index);

            // Return true if any one cell is selected
            for (int c = 0; c < model->GetColumnCount(parent); ++c)
            {
	            if (selection.Exists(model->GetIndex(index.GetRow(), c, parent)))
	            {
                    return true;
	            }
            }
        }
        else if (selectionType == FSelectionType::Column)
        {
	        // TODO: Not implemented
        }

        return selection.Exists(index);
    }

    void FItemSelectionModel::SetSelectionMode(FSelectionMode selectionMode)
    {
        if (this->selectionMode == selectionMode)
            return;

        this->selectionMode = selectionMode;

        ValidateSelection();

        m_OnSelectionChanged.Broadcast(this);
    }

    void FItemSelectionModel::ClearSelection()
    {
        selection.Clear();
        selectionStack.Clear();

        m_OnSelectionChanged.Broadcast(this);
    }

    void FItemSelectionModel::ValidateSelection()
    {
        if (selectionMode == FSelectionMode::None)
        {
            selection.Clear();
            selectionStack.Clear();
        }
        else if (selectionMode == FSelectionMode::Single && selection.GetSize() > 1)
        {
            auto first = *(selection.end() - 1);
            selection.Clear();
            selectionStack.Clear();

            selection.Add(first);
            selectionStack.Add(first);
        }
    }

} // namespace CE

