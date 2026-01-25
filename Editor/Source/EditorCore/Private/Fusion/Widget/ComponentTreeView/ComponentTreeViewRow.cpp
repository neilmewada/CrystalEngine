#include "EditorCore.h"

namespace CE::Editor
{

    ComponentTreeViewRow::ComponentTreeViewRow()
    {
        m_Padding = Vec4(1, 1, 1, 1) * 5;
    }

    void ComponentTreeViewRow::Construct()
    {
        Super::Construct();

        FBrush testImage = FBrush("/Engine/Resources/Icons/Warning");
        FBrush arrowImage = FBrush("/Engine/Resources/Icons/CaretDown");

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        Child(
            FAssignNew(FHorizontalStack, content)
            .Gap(5)
            .ContentVAlign(VAlign::Center)
            .ContentHAlign(HAlign::Left)
            (
                FAssignNew(FImageButton, arrow)
                .Image(arrowImage)
                .ImageWidth(10)
                .ImageHeight(10)
                .OnClicked([this]
                {
                    owner->ToggleExpand(this);
                })
                .VAlign(VAlign::Center)
                .Visible(false)
                .Style("ExpandCaretButton"),

                FAssignNew(FImage, icon)
                .Background(testImage)
                .Width(16)
                .Height(16)
                .VAlign(VAlign::Center),

                FAssignNew(FLabel, label)
                .Text("Label")
                .FontSize(fontSize + 1)
            )
        );
    }

    void ComponentTreeViewRow::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent())
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (mouseEvent->type == FEventType::MouseEnter)
            {
                isHovered = true;
                owner->ApplyStyle();
            }
            else if (mouseEvent->type == FEventType::MouseLeave)
            {
                isHovered = false;
                owner->ApplyStyle();
            }
            else if (mouseEvent->type == FEventType::MousePress)
            {
                owner->SelectItem(item);
            }
        }

	    Super::HandleEvent(event);
    }

    bool ComponentTreeViewRow::CheckParentRowRecursive(ComponentTreeViewRow* checkRow)
    {
        if (parentRow == checkRow)
            return true;
        return parentRow != nullptr && parentRow->CheckParentRowRecursive(checkRow);
    }

    ComponentTreeViewRow::Self& ComponentTreeViewRow::Indentation(f32 value)
    {
        content->Padding(Vec4(value, 0, 0, 0));
        return *this;
    }

    ComponentTreeViewRow::Self& ComponentTreeViewRow::ArrowExpanded(bool value)
    {
        arrow->GetImage()->Angle(value ? 0 : -90);
        return *this;
    }

    ComponentTreeViewRow::Self& ComponentTreeViewRow::ArrowVisible(bool value)
    {
        arrow->Visible(value);
        return *this;
    }

    ComponentTreeViewRow::Self& ComponentTreeViewRow::IconVisible(bool value)
    {
        icon->Enabled(value);
        return *this;
    }
}

