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
        FDockspace& dockspace = static_cast<FDockspace&>(widget);

        if (tabWellStyle && dockspace.GetDockTabWell()->IsOfType(tabWellStyle->GetWidgetClass()))
        {
            tabWellStyle->MakeStyle(*dockspace.GetDockTabWell());
        }

        Super::MakeStyle(widget);
    }

} // namespace CE

