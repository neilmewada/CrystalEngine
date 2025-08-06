#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FExpandableSectionStyle : public FStyle
    {
        CE_CLASS(FExpandableSectionStyle, FStyle)
    protected:

        FExpandableSectionStyle();

        virtual ~FExpandableSectionStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

    public:

        FIELD()
        FBrush headerBackground = Color::RGBA(47, 47, 47);

        FIELD()
        Vec4 headerCornerRadius;

        FIELD()
        Color headerBorderColor = Colors::Clear;

        FIELD()
        f32 headerBorder = 0;

        FIELD()
        FBrush contentBackground = Color::RGBA(36, 36, 36);

        FIELD()
        Vec4 contentCornerRadius;

        FIELD()
        Color contentBorderColor = Colors::Clear;

        FIELD()
        f32 contentBorder = 0;

    };
    
} // namespace CE

#include "FExpandableSectionStyle.rtti.h"
