#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FWindowControlButtonStyle : public FCustomButtonStyle
    {
        CE_CLASS(FWindowControlButtonStyle, FCustomButtonStyle)
    protected:

        FWindowControlButtonStyle();
        
    public:

        virtual ~FWindowControlButtonStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

    };
    
} // namespace CE

#include "FWindowControlButtonStyle.rtti.h"
