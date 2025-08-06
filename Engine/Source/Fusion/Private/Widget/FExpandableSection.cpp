#include "Fusion.h"

namespace CE
{

    FExpandableSection::FExpandableSection()
    {
        m_Expanded = true;
    }

    void FExpandableSection::Construct()
    {
        Super::Construct();

        FBrush downwardArrow = FBrush("/Engine/Resources/Icons/CaretDown");

        Child(
            FNew(FVerticalStack)
            .Gap(0)
            .VAlign(VAlign::Fill)
            .HAlign(HAlign::Fill)
            .Name("ExpandableSectionStack")
            (
                FAssignNew(FButton, header)
                .OnClicked([this]
                {
                    Expanded(!Expanded());

                    m_OnExpansionChanged.Broadcast(this);
                })
                .Background(Colors::Green)
                .ClipChildren(true)
                .Child(
                    FNew(FHorizontalStack)
                    .Gap(10)
                    .HAlign(HAlign::Fill)
                    .VAlign(VAlign::Center)
                    (
                        FAssignNew(FImage, arrowIcon)
                        .Background(downwardArrow)
                        .Width(10)
                        .Height(10)
                        .VAlign(VAlign::Center)
                        .HAlign(HAlign::Left)
                        .Margin(Vec4(0, 0, 5, 0))
                        .Angle(m_Expanded ? 0 : -90),

                        FAssignNew(FLabel, titleLabel)
                        .FontSize(11)
                        .Text("Section")
                        .VAlign(VAlign::Center),

                        FNew(FWidget)
                        .HAlign(HAlign::Fill)
                        .FillRatio(1.0f)
                    )
                )
                .Style(""),

                FAssignNew(FStyledWidget, content)
                .Background(Colors::Red)
                .Enabled(m_Expanded)
            )
        );

        header->ClearStyle();
        header->Background(Colors::Blue);
        header->Border(Colors::Clear, 0);
        header->CornerRadius(0);
    }

    void FExpandableSection::OnFusionPropertyModified(const CE::Name& propertyName)
    {
	    Super::OnFusionPropertyModified(propertyName);

        thread_local CE::Name expanded = "Expanded";

        if (propertyName == expanded)
        {
            arrowIcon->Angle(m_Expanded ? 0 : -90);
            content->Enabled(m_Expanded);
        }
    }

    FExpandableSection::Self& FExpandableSection::ExpandableContent(FWidget& widget)
    {
        content->Child(widget);
        return *this;
    }
}

