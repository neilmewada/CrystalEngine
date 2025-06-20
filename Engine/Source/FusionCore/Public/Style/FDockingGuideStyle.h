#pragma once

namespace CE
{
    class FDockingGuide;

    CLASS()
    class FUSIONCORE_API FDockingGuideStyle : public FStyledWidgetStyle
    {
        CE_CLASS(FDockingGuideStyle, FStyledWidgetStyle)
    protected:

        FDockingGuideStyle();
        
    public:

        virtual ~FDockingGuideStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

    };
    
} // namespace CE

#include "FDockingGuideStyle.rtti.h"
