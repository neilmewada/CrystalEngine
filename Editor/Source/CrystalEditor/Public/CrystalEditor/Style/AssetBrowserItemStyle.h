#pragma once

namespace CE
{
    CLASS()
    class CRYSTALEDITOR_API AssetBrowserItemStyle : public FSelectableButtonStyle
    {
        CE_CLASS(AssetBrowserItemStyle, FSelectableButtonStyle)
    protected:

        AssetBrowserItemStyle();
        
    public:

        virtual ~AssetBrowserItemStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;


        FIELD()
        FBrush folderBackground = Colors::Clear;

    };
    
} // namespace CE

#include "AssetBrowserItemStyle.rtti.h"
