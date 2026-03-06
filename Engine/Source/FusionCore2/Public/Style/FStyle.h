#pragma once

namespace CE
{
    class FWidget;

    ENUM()
    enum class FCursor
    {
        Default = 0,
        Arrow = 0,
        IBeam,
        Wait,
        CrossHair,
        WaitArrow,
        SizeTopLeft,
        SizeBottomRight = SizeTopLeft,
        SizeTopRight,
        SizeBottomLeft = SizeTopRight,
        SizeHorizontal,
        SizeVertical,
        SizeAll,
        No,
        Hand,
    };
    ENUM_CLASS(FCursor);

    CLASS(Abstract)
    class FUSIONCORE_API FStyle : public Object
    {
        CE_CLASS(FStyle, Object)
    public:

        virtual SubClass<FWidget> GetWidgetClass() const;

        virtual void MakeStyle(FWidget& widget) = 0;

    protected:

    };

} // namespace CE

#include "FStyle.rtti.h"