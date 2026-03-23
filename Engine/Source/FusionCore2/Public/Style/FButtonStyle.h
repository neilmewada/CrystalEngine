#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FButtonStyle : public FStyle
    {
        CE_CLASS(FButtonStyle, FStyle)
    protected:

        FButtonStyle();
        
    public:

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;


    };
    
} // namespace CE

#include "FButtonStyle.rtti.h"
