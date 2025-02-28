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
            .Height(100)
            .Style("SelectableButton")
            (
                FAssignNew(FLabel, titleLabel)
                .Text("Asset")
                .FontSize(9)
                .Foreground(Color::White())
                .HAlign(HAlign::Center)
                .VAlign(VAlign::Bottom)
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

