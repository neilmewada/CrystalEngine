#include "Fusion.h"

namespace CE
{

    FListWidgetStyle::FListWidgetStyle()
    {

    }

    FListWidgetStyle::~FListWidgetStyle()
    {
        
    }

    SubClass<FWidget> FListWidgetStyle::GetWidgetClass() const
    {
        return FListWidget::StaticType();
    }

    void FListWidgetStyle::MakeStyle(FWidget& widget)
    {
	    Super::MakeStyle(widget);

        FListWidget& listView = widget.As<FListWidget>();

        for (FListItemWidget* itemWidgetPtr : listView.itemWidgets)
        {
            FListItemWidget& itemWidget = *itemWidgetPtr;

            FBrush bg = itemBackground;
            Color border = itemBorderColor;

            if (itemWidget.IsSelected())
            {
                bg = selectedItemBackground;
                border = selectedItemBorderColor;
            }
            else if (itemWidget.IsHovered())
            {
                bg = hoveredItemBackground;
                border = hoveredItemBorderColor;
            }

            itemWidget
                .Background(bg)
                .Border(border, itemBorderWidth)
                ;
        }
    }

} // namespace CE

