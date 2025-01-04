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
            FNew(FHorizontalStack)
            .ContentHAlign(HAlign::Left)
            .ContentVAlign(VAlign::Fill)
            .VAlign(VAlign::Fill)
            .HAlign(HAlign::Fill)
            .Padding(Vec4(1.5f, 2, 1.5f, 2) * 3)
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
                .Margin(Vec4(0, 0, 5, 0))
                .Style("ExpandCaretButton"),

                FAssignNew(FImage, icon)
                .Width(11)
                .Height(11)
                .VAlign(VAlign::Center)
                .Enabled(false),

                FAssignNew(FLabel, label)
                .HAlign(HAlign::Left)
            )
        );

        ArrowExpanded(false);
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

