#pragma once

namespace CE
{
	ENUM()
	enum class FEasingType
	{
		Linear,
        EaseInQuad, EaseOutQuad, EaseInOutQuad,
        EaseInCubic, EaseOutCubic, EaseInOutCubic,
        EaseInQuart, EaseOutQuart, EaseInOutQuart,
        EaseInSine, EaseOutSine, EaseInOutSine,
        EaseInExpo, EaseOutExpo, EaseInOutExpo,
        EaseInCirc, EaseOutCirc, EaseInOutCirc,
        EaseInBack, EaseOutBack, EaseInOutBack,
        EaseInBounce, EaseOutBounce, EaseInOutBounce,
        EaseInElastic, EaseOutElastic, EaseInOutElastic,
        CubicBezier,   // Uses controlPoint1 / controlPoint2
	};
	ENUM_CLASS(FEasingType);

    STRUCT()
    struct FUSIONCORE_API FEasingCurve final
    {
        CE_STRUCT(FEasingCurve)
    public:

        FEasingCurve() = default;

        FEasingCurve(FEasingType type, Vec2 controlPoint1 = {}, Vec2 controlPoint2 = {}) 
    		: type(type)
    		, controlPoint1(controlPoint1)
    		, controlPoint2(controlPoint2)
		{}

        f32 Evaluate(f32 t) const;

        FIELD()
        FEasingType type = FEasingType::Linear;

        FIELD()
        Vec2 controlPoint1;

        FIELD()
        Vec2 controlPoint2;

    };

}

#include "FEasingCurve.rtti.h"