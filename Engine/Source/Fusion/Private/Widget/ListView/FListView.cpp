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

    void FListView::OnFusionPropertyModified(const CE::Name& propertyName)
    {
        Super::OnFusionPropertyModified(propertyName);

        static CE::Name modelProperty = "Model";

        if (propertyName == modelProperty && m_Model)
        {
            m_Model->listView = this;
            m_Model->OnListViewAssigned();
        }
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

        m_OnSelectionChanged.Broadcast(this);

        ApplyStyle();
    }

    bool FListView::IsRowSelected(int index)
    {
        return selectedRows.Exists(index);
    }

    void FListView::OnModelUpdate()
    {
        container->UpdateRows();
    }
}

