#include "FusionCore.h"

namespace CE
{

	f32 FEasingCurve::Evaluate(f32 t) const
	{
		// Clamp for non-overshoot curves; overshoot curves (Back, Elastic) naturally exceed [0,1]
		t = Math::Clamp01(t);

		switch (type)
		{
		default:
		case FEasingType::Linear:
			return t;

		// --- Quad ---
		case FEasingType::EaseInQuad:
			return t * t;
		case FEasingType::EaseOutQuad:
			return t * (2.0f - t);
		case FEasingType::EaseInOutQuad:
			return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;

		// --- Cubic ---
		case FEasingType::EaseInCubic:
			return t * t * t;
		case FEasingType::EaseOutCubic:
		{
			f32 t1 = t - 1.0f;
			return t1 * t1 * t1 + 1.0f;
		}
		case FEasingType::EaseInOutCubic:
			return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;

		// --- Quart ---
		case FEasingType::EaseInQuart:
			return t * t * t * t;
		case FEasingType::EaseOutQuart:
		{
			f32 t1 = t - 1.0f;
			return 1.0f - t1 * t1 * t1 * t1;
		}
		case FEasingType::EaseInOutQuart:
		{
			f32 t1 = t - 1.0f;
			return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - 8.0f * t1 * t1 * t1 * t1;
		}

		// --- Sine ---
		case FEasingType::EaseInSine:
			return 1.0f - Math::Cos(t * Math::PI * 0.5f);
		case FEasingType::EaseOutSine:
			return Math::Sin(t * Math::PI * 0.5f);
		case FEasingType::EaseInOutSine:
			return 0.5f * (1.0f - Math::Cos(Math::PI * t));

		// --- Expo ---
		case FEasingType::EaseInExpo:
			return t == 0.0f ? 0.0f : Math::Pow(2.0f, 10.0f * t - 10.0f);
		case FEasingType::EaseOutExpo:
			return t == 1.0f ? 1.0f : 1.0f - Math::Pow(2.0f, -10.0f * t);
		case FEasingType::EaseInOutExpo:
			if (t == 0.0f) return 0.0f;
			if (t == 1.0f) return 1.0f;
			return t < 0.5f
				? Math::Pow(2.0f, 20.0f * t - 10.0f) * 0.5f
				: (2.0f - Math::Pow(2.0f, -20.0f * t + 10.0f)) * 0.5f;

		// --- Circ ---
		case FEasingType::EaseInCirc:
			return 1.0f - Math::Sqrt(1.0f - t * t);
		case FEasingType::EaseOutCirc:
		{
			f32 t1 = t - 1.0f;
			return Math::Sqrt(1.0f - t1 * t1);
		}
		case FEasingType::EaseInOutCirc:
		{
			f32 t1 = 2.0f * t - 2.0f;
			return t < 0.5f
				? 0.5f * (1.0f - Math::Sqrt(1.0f - 4.0f * t * t))
				: 0.5f * (Math::Sqrt(1.0f - t1 * t1) + 1.0f);
		}

		// --- Back (overshoots slightly past 1) ---
		case FEasingType::EaseInBack:
		{
			constexpr f32 c = 1.70158f;
			return t * t * ((c + 1.0f) * t - c);
		}
		case FEasingType::EaseOutBack:
		{
			constexpr f32 c = 1.70158f;
			f32 t1 = t - 1.0f;
			return t1 * t1 * ((c + 1.0f) * t1 + c) + 1.0f;
		}
		case FEasingType::EaseInOutBack:
		{
			constexpr f32 c1 = 1.70158f;
			constexpr f32 c2 = c1 * 1.525f;
			f32 t1 = 2.0f * t - 2.0f;
			return t < 0.5f
				? (2.0f * t * 2.0f * t * ((c2 + 1.0f) * 2.0f * t - c2)) * 0.5f
				: (t1 * t1 * ((c2 + 1.0f) * t1 + c2) + 2.0f) * 0.5f;
		}

		// --- Bounce ---
		case FEasingType::EaseOutBounce:
		{
			constexpr f32 n = 7.5625f;
			constexpr f32 d = 2.75f;
			if (t < 1.0f / d)
				return n * t * t;
			else if (t < 2.0f / d)
			{
				t -= 1.5f / d;
				return n * t * t + 0.75f;
			}
			else if (t < 2.5f / d)
			{
				t -= 2.25f / d;
				return n * t * t + 0.9375f;
			}
			else
			{
				t -= 2.625f / d;
				return n * t * t + 0.984375f;
			}
		}
		case FEasingType::EaseInBounce:
		{
			// EaseIn = 1 - EaseOut(1 - t)
			f32 t1 = 1.0f - t;
			FEasingCurve out(FEasingType::EaseOutBounce);
			return 1.0f - out.Evaluate(t1);
		}
		case FEasingType::EaseInOutBounce:
		{
			FEasingCurve out(FEasingType::EaseOutBounce);
			return t < 0.5f
				? (1.0f - out.Evaluate(1.0f - 2.0f * t)) * 0.5f
				: (1.0f + out.Evaluate(2.0f * t - 1.0f)) * 0.5f;
		}

		// --- Elastic (overshoots, spring-like) ---
		case FEasingType::EaseInElastic:
		{
			if (t == 0.0f) return 0.0f;
			if (t == 1.0f) return 1.0f;
			constexpr f32 c = (2.0f * Math::PI) / 3.0f;
			return -Math::Pow(2.0f, 10.0f * t - 10.0f) * Math::Sin((t * 10.0f - 10.75f) * c);
		}
		case FEasingType::EaseOutElastic:
		{
			if (t == 0.0f) return 0.0f;
			if (t == 1.0f) return 1.0f;
			constexpr f32 c = (2.0f * Math::PI) / 3.0f;
			return Math::Pow(2.0f, -10.0f * t) * Math::Sin((t * 10.0f - 0.75f) * c) + 1.0f;
		}
		case FEasingType::EaseInOutElastic:
		{
			if (t == 0.0f) return 0.0f;
			if (t == 1.0f) return 1.0f;
			constexpr f32 c = (2.0f * Math::PI) / 4.5f;
			return t < 0.5f
				? -(Math::Pow(2.0f, 20.0f * t - 10.0f) * Math::Sin((20.0f * t - 11.125f) * c)) * 0.5f
				:  (Math::Pow(2.0f, -20.0f * t + 10.0f) * Math::Sin((20.0f * t - 11.125f) * c)) * 0.5f + 1.0f;
		}

		// --- Cubic Bezier ---
		case FEasingType::CubicBezier:
		{
			// Newton-Raphson solve for the parameter s such that BezierX(s) == t,
			// then return BezierY(s). P0=(0,0), P3=(1,1); P1=controlPoint1, P2=controlPoint2.
			auto BezierCoord = [](f32 p1, f32 p2, f32 s) -> f32
			{
				// Cubic Bernstein: 3*(1-s)^2*s*p1 + 3*(1-s)*s^2*p2 + s^3
				f32 s1 = 1.0f - s;
				return 3.0f * s1 * s1 * s * p1 + 3.0f * s1 * s * s * p2 + s * s * s;
			};
			auto BezierCoordDerivative = [](f32 p1, f32 p2, f32 s) -> f32
			{
				f32 s1 = 1.0f - s;
				return 3.0f * s1 * s1 * p1 + 6.0f * s1 * s * (p2 - p1) + 3.0f * s * s * (1.0f - p2);
			};

			// Solve BezierX(s) = t via Newton-Raphson (8 iterations is sufficient)
			f32 s = t;
			for (int i = 0; i < 8; ++i)
			{
				f32 bx   = BezierCoord(controlPoint1.x, controlPoint2.x, s) - t;
				f32 bxd  = BezierCoordDerivative(controlPoint1.x, controlPoint2.x, s);
				if (Math::Abs(bxd) < 1e-6f) break;
				s -= bx / bxd;
				s  = Math::Clamp01(s);
			}

			return BezierCoord(controlPoint1.y, controlPoint2.y, s);
		}
		}
	}

}
