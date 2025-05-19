#include "Fusion.h"

namespace CE
{

    FListViewStyle::FListViewStyle()
    {

    }

    FListViewStyle::~FListViewStyle()
    {
        
    }

    SubClass<FWidget> FListViewStyle::GetWidgetClass() const
    {
        return FListView::StaticType();
    }

    void FListViewStyle::MakeStyle(FWidget& widget)
    {
	    Super::MakeStyle(widget);

        FListView& listView = widget.As<FListView>();

        for (int i = 0; i < listView.GetVisibleRowCount(); ++i)
        {
            FListViewRow* row = listView.GetVisibleRow(i);
            if (!row->Enabled())
                continue;

            FBrush bg = row->IsAlternate() ? alternateItemBackground : itemBackground;
            Color border = itemBorderColor;

            if (row->IsSelected())
            {
                bg = selectedItemBackground;
                border = selectedItemBorderColor;
            }
            else if (row->IsHovered())
            {
                bg = hoveredItemBackground;
                border = hoveredItemBorderColor;
            }

            (*row)
                .Background(bg)
                .Border(border, itemBorderWidth)
            ;
        }
    }

} // namespace CE

