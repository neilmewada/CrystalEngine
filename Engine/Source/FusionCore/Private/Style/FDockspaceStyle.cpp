#include "FusionCore.h"

namespace CE
{

    FDockspaceStyle::FDockspaceStyle()
    {
        tabWellStyle = CreateDefaultSubobject<FDockTabWellStyle>("DockTabWellStyle");
    }

    FDockspaceStyle::~FDockspaceStyle()
    {
        
    }

    SubClass<FWidget> FDockspaceStyle::GetWidgetClass() const
    {
        return FDockspace::StaticClass();
    }

    void FDockspaceStyle::MakeStyle(FWidget& widget)
    {
        Super::MakeStyle(widget);

        FDockspace& dockspace = static_cast<FDockspace&>(widget);

        if (tabWellStyle)
        {
            dockspace.IterateTabWellsRecursively([&](FDockTabWell& tabWell)
                {
					if (tabWell.IsOfType(tabWellStyle->GetWidgetClass()))
                    {
	                    tabWellStyle->MakeStyle(tabWell);
                    }
                });
        }
    }

} // namespace CE

