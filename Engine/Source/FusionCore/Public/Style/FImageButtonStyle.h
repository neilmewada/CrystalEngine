#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FImageButtonStyle : public FStyle
    {
        CE_CLASS(FImageButtonStyle, FStyle)
    protected:

        FImageButtonStyle();

    public: // - Public API -

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

    public: // - Properties - 

        FIELD()
        Color tintColor = Colors::White;

        FIELD()
        Color hoverTintColor = Colors::White;

        FIELD()
        Color pressedTintColor = Colors::White;

    };
    
}

#include "FImageButtonStyle.rtti.h"
