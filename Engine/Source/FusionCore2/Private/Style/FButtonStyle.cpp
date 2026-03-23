#include "FusionCore.h"

namespace CE
{

    FButtonStyle::FButtonStyle()
    {

    }

    SubClass<FWidget> FButtonStyle::GetWidgetClass() const
    {
	    return FButton::StaticClass();
    }

    void FButtonStyle::MakeStyle(FWidget& widget)
    {
        FButton& button = widget.As<FButton>();


    }
} // namespace CE

