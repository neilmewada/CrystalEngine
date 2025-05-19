#include "Fusion.h"

namespace CE
{

	FListWidget::FListWidget()
	{

	}

    void FListWidget::Construct()
    {
        Super::Construct();

        Child(
            FAssignNew(FScrollBox, scrollBox)
            .VerticalScroll(true)
            .HorizontalScroll(false)
            .Child(
                FAssignNew(FListWidgetContainer, content)
                .HAlign(HAlign::Fill)
                .Padding(Vec4(1, 1, 1, 1) * 10)
            )
            .HAlign(HAlign::Fill)
            .Name("ListViewScrollBox")
        );
    }

    void FListWidget::OnPostComputeLayout()
    {
	    Super::OnPostComputeLayout();

    }

    void FListWidget::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        thread_local const CE::Name ItemListName = "ItemList";

        if (propertyName == ItemListName)
        {
            RegenerateRows();
        }
    }

    void FListWidget::SelectItem(int index)
    {
        if (index < 0 || index >= itemWidgets.GetSize())
            return;

        itemWidgets[index]->Select();
    }

    FListItemWidget* FListWidget::GetSelectedItem()
    {
        if (selectedItems.IsEmpty())
            return nullptr;
        return selectedItems[0];
    }

    int FListWidget::GetSelectedItemIndex()
    {
        if (selectedItems.IsEmpty())
            return -1;
	    return itemWidgets.IndexOf(selectedItems[0]);
    }


    void FListWidget::OnItemSelected(FListItemWidget* selectedItem)
    {
        if (m_SelectionMode == FSelectionMode::None)
            return;

        selectedItems.Clear();

        for (FListItemWidget* item : itemWidgets)
        {
	        if (item != selectedItem && item->IsSelected())
	        {
                item->itemState &= ~FListItemState::Selected;
	        }
            else if (item == selectedItem && !item->IsSelected())
            {
                item->itemState |= FListItemState::Selected;
                selectedItems.Add(item);
            }
        }

        ApplyStyle();

        m_OnSelectionChanged(this);
    }

    void FListWidget::RegenerateRows()
    {
        // Destroy previous FListItemWidget's

        // TODO: Optimize row generation by using recycling, culling, etc

        while (content->GetChildCount() > 0)
        {
            content->GetChild(0)->BeginDestroy();
        }

        itemWidgets.Clear();

        if (m_GenerateRowDelegate.IsValid())
        {
            for (FListItem* item : m_ItemList)
            {
                FListItemWidget& itemWidget = m_GenerateRowDelegate(item, this);
                itemWidget.listView = this;
                content->AddChild(&itemWidget);
                itemWidgets.Add(&itemWidget);
            }
        }

        scrollBox->ClampTranslation();
        MarkLayoutDirty();
    }

    FListWidget::Self& FListWidget::GenerateRowDelegate(const GenerateRowCallback& callback)
    {
        m_GenerateRowDelegate = callback;
        RegenerateRows();
        return *this;
    }

}

