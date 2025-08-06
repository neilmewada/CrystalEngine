#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FCheckboxStyle : public FCustomButtonStyle
    {
        CE_CLASS(FCheckboxStyle, FCustomButtonStyle)
    protected:

        FCheckboxStyle();

    public: // - Public API -

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

        FIELD()
        Color foregroundDisabledColor = Colors::White;

        FIELD()
        Color foregroundColor = Colors::White;

        FIELD()
        Color foregroundHoverColor = Colors::White;

        FIELD()
        Color foregroundPressedColor = Colors::White;
    };
    
}

#include "FCheckboxStyle.rtti.h"
