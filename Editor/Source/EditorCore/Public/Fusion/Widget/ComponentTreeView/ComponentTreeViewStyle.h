#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API ComponentTreeViewStyle : public FStyledWidgetStyle
    {
        CE_CLASS(ComponentTreeViewStyle, FStyledWidgetStyle)
    protected:

        ComponentTreeViewStyle();
        
    public:

        virtual ~ComponentTreeViewStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

        FIELD()
        FBrush rowBackground = Colors::Clear;

        FIELD()
        FBrush rowAlternateBackground = Color::RGBA(21, 21, 21);

        FIELD()
        FBrush rowHoverBackground = Color::RGBA(36, 36, 36);

        FIELD()
        FBrush rowSelectedBackground = Color::RGBA(0, 112, 224);

    };
    
} // namespace CE

#include "ComponentTreeViewStyle.rtti.h"
