#pragma once

namespace CE
{
    class FWidget;
    class FStyleSet;

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

        virtual void MakeStyle(FWidget& widget) const = 0;

        Ref<FStyleSet> GetParent() const { return parent.Lock(); }

    protected:



    private:

        FIELD()
        WeakRef<FStyleSet> parent;

        friend class FStyleSet;
    };

} // namespace CE

#include "FStyle.rtti.h"