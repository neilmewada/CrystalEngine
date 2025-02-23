#pragma once

namespace CE
{

    CLASS()
    class FUSIONCORE_API FSelectableButtonStyle : public FStyle
    {
        CE_CLASS(FSelectableButtonStyle, FStyle)
    protected:

        FSelectableButtonStyle();
        
    public:

        virtual ~FSelectableButtonStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;


        FIELD()
        FShapeType backgroundShape = FShapeType::RoundedRect;

        FIELD()
        Vec4 cornerRadius = Vec4();

        FIELD()
        FBrush background{};

        FIELD()
        FBrush disabledBackground{};

        FIELD()
        FBrush hoveredBackground{};

        FIELD()
        FBrush selectedBackground{};

        FIELD()
        f32 borderWidth = 1.0f;

        FIELD()
        Color borderColor{};

        FIELD()
        Color disabledBorderColor{};

        FIELD()
        Color hoveredBorderColor{};

        FIELD()
        Color selectedBorderColor{};

    };
    
} // namespace CE

#include "FSelectableButtonStyle.rtti.h"
