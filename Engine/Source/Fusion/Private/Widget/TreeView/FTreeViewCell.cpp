#include "Fusion.h"

namespace CE
{

    FTreeViewCell::FTreeViewCell()
    {
        m_ClipChildren = true;
    }

    void FTreeViewCell::Construct()
    {
        Super::Construct();

        FBrush downwardArrow = FBrush("/Engine/Resources/Icons/CaretDown");

        Child(
            FAssignNew(FHorizontalStack, contentStack)
            .ContentHAlign(HAlign::Left)
            .ContentVAlign(VAlign::Fill)
            .VAlign(VAlign::Fill)
            .HAlign(HAlign::Fill)
            .Padding(Vec4(1.5f, 1.5f, 1.5f, 1.5f) * 3)
            (
                FAssignNew(FImageButton, arrowIcon)
                .Image(downwardArrow)
                .OnClicked([this]
                {
                    m_OnToggleExpansion();
                })
                .Width(10)
                .Height(10)
                .VAlign(VAlign::Center)
                .Enabled(true)
                .Visible(false)
                .Margin(Vec4(0, 0, 7.5f, 0))
                .Style("ExpandCaretButton"),

                FAssignNew(FImage, icon)
                .Width(12)
                .Height(12)
                .VAlign(VAlign::Center)
                .Margin(Vec4(0, 0, 5, 0))
                .Enabled(false),

                FAssignNew(FLabel, label)
                .HAlign(HAlign::Left)
                .VAlign(VAlign::Center)
            )
        );

        ArrowExpanded(false);
    }

    void FTreeViewCell::StartEditing()
    {
        if (textInput)
        {
            contentStack->RemoveChild(textInput.Get());
            textInput->BeginDestroy();
            textInput = nullptr;
        }

        FAssignNew(FTextInput, textInput)
        .Text(label->Text())
        .FontSize(label->FontSize())
        .HAlign(HAlign::Left)
    	.VAlign(VAlign::Center);

        textInput->FontSize(label->FontSize());

        contentStack->AddChild(textInput.Get());

        label->Enabled(false);
        textInput->Enabled(true);

        textInput->StartEditing(true, true);

        textInput->OnTextEditingFinished([this](FTextInput*)
            {
				String text = textInput->Text();
				label->Text(text);
                contentStack->RemoveChild(textInput.Get());
                textInput->QueueDestroy();

                label->Enabled(true);
                this->textInput = nullptr;

                m_OnLabelEdited.Invoke(*this, text);
            });
    }

    FTreeViewCell::Self& FTreeViewCell::IconWidth(f32 value)
    {
        icon->Width(value);
        return *this;
    }

    FTreeViewCell::Self& FTreeViewCell::IconHeight(f32 value)
    {
        icon->Height(value);
        return *this;
    }

    bool FTreeViewCell::ArrowExpanded()
    {
        return abs(arrowIcon->Angle()) < 1;
    }

    FTreeViewCell::Self& FTreeViewCell::ArrowExpanded(bool expanded)
    {
        arrowIcon->Angle(expanded ? 0 : -90);
        return *this;
    }

}

