#include "Fusion.h"

namespace CE
{

    FListView::FListView()
    {
        m_SelectionMode = FSelectionMode::Single;
    }

    void FListView::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FScrollBox, scrollBox)
            .VerticalScroll(true)
            .HorizontalScroll(false)
            .OnScrollValueChanged([this]
            {
                container->UpdateRows();
            })
            .Child(
                FAssignNew(FListViewContainer, container)
                .ListView(this)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Top)
            )
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
        );
    }

    int FListView::GetVisibleRowCount()
    {
        return container->children.GetCount();
    }

    FListViewRow* FListView::GetVisibleRow(int index)
    {
        return container->children[index];
    }

    void FListView::SelectRow(FListViewRow* row, bool additional)
    {
        if (!row || m_SelectionMode == FSelectionMode::None)
            return;
        if (row->rowIndex < 0)
            return;

        if (m_SelectionMode == FSelectionMode::Single || !additional)
        {
            selectedRows.Clear();
        }

        selectedRows.Add(row->rowIndex);

        container->UpdateRows();
        container->OnSelectionChanged();

        ApplyStyle();
    }

    bool FListView::IsRowSelected(int index)
    {
        return selectedRows.Exists(index);
    }
}

