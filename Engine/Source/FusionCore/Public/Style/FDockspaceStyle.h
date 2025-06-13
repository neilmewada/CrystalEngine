#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockspaceStyle : public FStyledWidgetStyle
    {
        CE_CLASS(FDockspaceStyle, FStyledWidgetStyle)
    protected:

        FDockspaceStyle();
        
    public:

        virtual ~FDockspaceStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

        FIELD()
        Ref<FDockTabWellStyle> tabWellStyle;


    };
    
} // namespace CE

#include "FDockspaceStyle.rtti.h"
