#include "FusionCore.h"

namespace CE
{

    FDockTabWellStyle::FDockTabWellStyle()
    {
        background = FBrush();
    }

    FDockTabWellStyle::~FDockTabWellStyle()
    {
        
    }

    SubClass<FWidget> FDockTabWellStyle::GetWidgetClass() const
    {
        return FDockTabWell::StaticClass();
    }

    void FDockTabWellStyle::MakeStyle(FWidget& widget)
    {
        FDockTabWell& dockTabWell = static_cast<FDockTabWell&>(widget);

        dockTabWell
        .Background(background)
        .Border(borderColor, borderWidth)
        ;

        for (Ref<FDockTabItem> dockTabItem : dockTabWell.tabItems)
        {
            (*dockTabItem)
            .BorderColor(dockTabItem->IsActive()
                ? tabActiveBorderColor
                : (dockTabItem->IsHovered() ? tabHoverBorderColor : tabBorderColor))
            .BorderWidth(tabBorderWidth)
            .CornerRadius(tabCornerRadius)
            .Background(dockTabItem->IsActive()
                ? tabActiveBackground
                : (dockTabItem->IsHovered() ? tabHoverBackground : tabBackground))
            ;
        }
    }
} // namespace CE

