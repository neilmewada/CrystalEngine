#include "FusionCore.h"

namespace CE
{

    FComboBox::FComboBox()
    {
        m_Padding = Vec4(7.5f, 5, 10, 5);
    }

    void FComboBox::ApplyStyle()
    {
	    Super::ApplyStyle();

        ItemStyle(m_ItemStyle);
    }

    void FComboBox::SelectItem(FComboBoxItem* item)
    {
        selectedItem = item;
        selectionText->Text(selectedItem->Text());

        for (int i = 0; i < popupContent->GetChildCount(); ++i)
        {
            Ref<FWidget> child = popupContent->GetChild(i);
            if (!child->IsOfType<FComboBoxItem>())
                continue;

            FComboBoxItem* childItem = static_cast<FComboBoxItem*>(child.Get());
            if (childItem == item)
            {
                childItem->state |= FComboBoxItemState::Selected;
            }
            else
            {
                childItem->state &= ~FComboBoxItemState::Selected;
            }
        }

        popupMenu->ClosePopup();

        ApplyStyle();
    }

    void FComboBox::SelectItem(int index)
    {
        if (index < 0 || index >= popupContent->GetChildCount())
            return;

        Ref<FWidget> child = popupContent->GetChild(index);
        if (!child->IsOfType<FComboBoxItem>())
            return;

        SelectItem(static_cast<FComboBoxItem*>(child.Get()));
    }

    int FComboBox::GetItemCount() const
    {
        return popupContent->GetChildCount();
    }

    FComboBoxItem* FComboBox::GetItem(int index)
    {
        if (index < 0 || index >= popupContent->GetChildCount())
            return nullptr;
        if (!popupContent->GetChild(index)->IsOfType<FComboBoxItem>())
            return nullptr;

        return static_cast<FComboBoxItem*>(popupContent->GetChild(index).Get());
    }

    int FComboBox::GetSelectedItemIndex()
    {
        if (selectedItem == nullptr)
            return -1;

        for (int i = 0; i < popupContent->GetChildCount(); ++i)
        {
            if (popupContent->GetChild(i) == selectedItem)
                return i;
        }

        return -1;
    }

    void FComboBox::OnBeforeDestroy()
    {
        Super::OnBeforeDestroy();

        if (popupMenu != nullptr)
        {
            popupMenu->ClosePopup();
            popupMenu = nullptr;
        }
    }


    void FComboBox::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        static const CE::Name ItemsName = "Items";

        if (propertyName == ItemsName)
        {
            int selectedIndex = GetSelectedItemIndex();

            DestroyAllItems();

            for (const String& item : m_Items)
            {
                AddItem(item);
            }

            if (selectedIndex >= 0 && selectedIndex < m_Items.GetSize())
            {
                SelectItem(selectedIndex);
            }
        }
    }

    void FComboBox::SelectItemInternal(FComboBoxItem* item)
    {
        SelectItem(item);

        for (int i = 0; i < popupContent->GetChildCount(); ++i)
        {
	        if (popupContent->GetChild(i) == item)
	        {
                m_OnSelectionChanged(this);
		        break;
	        }
        }
    }

    void FComboBox::AddItem(FComboBoxItem& item)
    {
        item.comboBox = this;

        if (m_ItemStyle)
        {
	        item.Style(m_ItemStyle);
        }

        popupMenu->Content(item);
    }

    void FComboBox::AddItem(const String& text)
    {
        FComboBoxItem& item = FNewOwned(FComboBoxItem, popupContent);
        AddItem(item);
        item.Text(text);
    }

    void FComboBox::Construct()
    {
        Super::Construct();

        FBrush downwardArrow = FBrush("/Engine/Resources/Icons/DownwardArrow");

        FAssignNew(FComboBoxPopup, popupMenu)
    		.BlockInteraction(false)
    		.AutoClose(true)
    		.MinWidth(60)
    		.MinHeight(30);

        popupMenu->SetContextWidget(this);

        popupContent = popupMenu->GetContentStack();

        Child(
            FNew(FHorizontalStack)
            .ContentVAlign(VAlign::Center)
            (
                FAssignNew(FLabel, selectionText)
                .Text("[Select]")
                .FontSize(10)
                .Name("SelectionText")
                .Margin(Vec4(0, 0, 5, 0)),

                FNew(FWidget)
                .FillRatio(1.0f),

                FNew(FImage)
                .Background(downwardArrow)
                .Width(10)
                .Height(10)
            )
        );
    }

    void FComboBox::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent())
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MouseEnter && event->sender == this)
            {
                if (!EnumHasFlag(state, FComboBoxState::Hovered))
                {
                    state |= FComboBoxState::Hovered;
                    ApplyStyle();
                }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MouseLeave && event->sender == this)
            {
                if (EnumHasFlag(state, FComboBoxState::Hovered))
                {
                    state &= ~FComboBoxState::Hovered;
                    ApplyStyle();
                }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MousePress && mouseEvent->buttons == MouseButtonMask::Left)
            {
                if (!EnumHasFlag(state, FComboBoxState::Pressed))
                {
                    state |= FComboBoxState::Pressed;
                    ApplyStyle();
                }
                event->Consume(this);
            }
            else if (mouseEvent->type == FEventType::MouseRelease && mouseEvent->buttons == MouseButtonMask::Left)
            {
                if (EnumHasFlag(state, FComboBoxState::Pressed))
                {
                    state &= ~FComboBoxState::Pressed;

                    if (mouseEvent->isInside)
                    {
                        Vec2 popupPosition = globalPosition;
                        popupPosition.y += computedSize.height;
                        popupMenu->MinWidth(GetComputedSize().width);

                        GetContext()->PushLocalPopup(popupMenu.Get(), popupPosition, Vec2(), computedSize);
                    }

                    ApplyStyle();
                }
                event->Consume(this);
            }
        }

	    Super::HandleEvent(event);
    }

    FComboBox& FComboBox::ItemStyle(FComboBoxItemStyle* value)
    {
        m_ItemStyle = value;
        if (!popupContent)
            return *this;

        for (int i = 0; i < popupContent->GetChildCount(); ++i)
        {
            Ref<FWidget> child = popupContent->GetChild(i);
            child->Style(m_ItemStyle);
        }

        return *this;
    }

    FComboBox& FComboBox::ItemStyle(FStyleSet* styleSet, const CE::Name& styleKey)
    {
        if (!styleSet)
            return *this;

        FStyle* style = styleSet->FindStyle(styleKey);
        if (!style || !style->IsOfType<FComboBoxItemStyle>())
            return *this;

        return ItemStyle(static_cast<FComboBoxItemStyle*>(style));
    }

    void FComboBox::DestroyAllItems()
    {
        Array<FWidget*> itemsToDestroy{};
        itemsToDestroy.Reserve(popupContent->GetChildCount());

        for (int i = 0; i < popupContent->GetChildCount(); ++i)
        {
            Ref<FWidget> child = popupContent->GetChild(i);
            if (!child->IsOfType<FComboBoxItem>())
                continue;

            itemsToDestroy.Add(child.Get());
        }

        for (FWidget* widget : itemsToDestroy)
        {
            widget->BeginDestroy();
        }
    }

}

