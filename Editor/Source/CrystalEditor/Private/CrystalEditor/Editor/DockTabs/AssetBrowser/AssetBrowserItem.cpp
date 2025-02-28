#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowserItem::AssetBrowserItem()
    {
        m_ClipChildren = true;
    }

    void AssetBrowserItem::Construct()
    {
        Super::Construct();

        (*this)
            .Width(80)
            .Height(110)
            .Style("SelectableButton")
            (
                FNew(FVerticalStack)
                .Gap(2)
                .ContentHAlign(HAlign::Fill)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                (
                    FNew(FStyledWidget)
                    .Background(Color::Black())
                    .BackgroundShape(FRoundedRectangle(5, 5, 0, 0))
                    .Height(70),

                    FAssignNew(FLabel, titleLabel)
                    .Text("Asset")
                    .FontSize(9)
                    .WordWrap(FWordWrap::Normal)
                    .Foreground(Color::White())
                    .HAlign(HAlign::Left)
                    .VAlign(VAlign::Bottom)
                    .Margin(Vec4(2.5f, 0, 0, 0))
                    .Height(20),

                    FNew(FWidget)
                    .FillRatio(1.0f),

                    FAssignNew(FLabel, subtitleLabel)
                    .Text("Asset Type")
                    .FontSize(7)
                    .Foreground(Color::White().WithAlpha(0.5f))
                    .HAlign(HAlign::Left)
                    .VAlign(VAlign::Bottom)
                    .Margin(Vec4(2.5f, 0, 0, 0))
                )
            )
        ;
    }

    void AssetBrowserItem::HandleEvent(FEvent* event)
    {
        if (auto owner = m_Owner.Lock())
        {
            if (event->IsMouseEvent() && !IsInteractionDisabled() && IsVisible())
            {
                FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

                if (mouseEvent->type == FEventType::MousePress && mouseEvent->sender == this)
                {
                    if (mouseEvent->buttons == MouseButtonMask::Left)
                    {
                        if (!EnumHasFlag(mouseEvent->keyModifiers, KeyModifier::Ctrl))
                        {
                            owner->DeselectAll();
                        }
                    }
                    else if (mouseEvent->buttons == MouseButtonMask::Right)
                    {

                    }
                }
            }
        }

        Super::HandleEvent(event);
    }


    void AssetBrowserItem::OnSelected()
    {
        Super::OnSelected();


    }

    void AssetBrowserItem::OnDeselected()
    {
        Super::OnDeselected();

    }

    
}

