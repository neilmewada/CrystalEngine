#pragma once

namespace CE
{

    template<typename T>
    struct FAnimatable
    {
        static T Lerp(const T& a, const T& b, f32 t) { return a + (b - a) * Math::Clamp01(t); }

        static T LerpUnclamped(const T& a, const T& b, f32 t) { return a + (b - a) * t; }

        // Identity/zero value — needed for spring initial velocity
        static T Identity() { return {}; }

        // Spring arithmetic helpers
        static T Add(const T& a, const T& b) { return a + b; }
        static T Scale(const T& a, f32 s) { return a * s; }

        static f32 SquaredMagnitude(const T& v)
        {
            if constexpr (requires { v.GetSqrMagnitude(); })
                return v.GetSqrMagnitude();
            else
                return static_cast<f32>(v * v);
        }
    };

    template<>
    struct FAnimatable<Color>
    {
        static Color Lerp(const Color& a, const Color& b, f32 t) { return Color::Lerp(a, b, Math::Clamp01(t)); }

        static Color LerpUnclamped(const Color& a, const Color& b, f32 t) { return Color::Lerp(a, b, t); }

        // Identity/zero value — needed for spring initial velocity
        static Color Identity() { return {}; }

        // Spring arithmetic helpers
        static Color Add(const Color& a, const Color& b)
        {
            return Color(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
        }

        static Color Scale(const Color& a, f32 s) { return a * s; }

        static f32 SquaredMagnitude(const Color& v)
        {
            return v.r * v.r + v.g * v.g + v.b * v.b + v.a * v.a;
        }
    };

    template<>
    struct FAnimatable<FAffineTransform>
    {
        static FAffineTransform Lerp(const FAffineTransform& a, const FAffineTransform& b, f32 t)
        {
            return LerpUnclamped(a, b, Math::Clamp01(t));
        }

        static FAffineTransform LerpUnclamped(const FAffineTransform& a, const FAffineTransform& b, f32 t)
        {
            // Decompose a
            const f32 sxA    = Math::Sqrt(a.m00 * a.m00 + a.m10 * a.m10);
            const f32 thetaA = std::atan2(a.m10, a.m00);
            const f32 cosA   = sxA > 1e-8f ? a.m00 / sxA : 1.0f;
            const f32 sinA   = sxA > 1e-8f ? a.m10 / sxA : 0.0f;
            const f32 shearA = cosA * a.m01 + sinA * a.m11;
            const f32 syA    = -sinA * a.m01 + cosA * a.m11;

            // Decompose b
            const f32 sxB    = Math::Sqrt(b.m00 * b.m00 + b.m10 * b.m10);
            const f32 thetaB = std::atan2(b.m10, b.m00);
            const f32 cosB   = sxB > 1e-8f ? b.m00 / sxB : 1.0f;
            const f32 sinB   = sxB > 1e-8f ? b.m10 / sxB : 0.0f;
            const f32 shearB = cosB * b.m01 + sinB * b.m11;
            const f32 syB    = -sinB * b.m01 + cosB * b.m11;

            // Lerp each component
            const f32 tx    = a.tx   + (b.tx    - a.tx)   * t;
            const f32 ty    = a.ty   + (b.ty    - a.ty)   * t;
            const f32 sx    = sxA   + (sxB     - sxA)    * t;
            const f32 sy    = syA   + (syB     - syA)    * t;
            const f32 theta = thetaA + (thetaB  - thetaA) * t;
            const f32 shear = shearA + (shearB  - shearA) * t;

            // Recompose
            const f32 cosT = std::cos(theta);
            const f32 sinT = std::sin(theta);

            FAffineTransform out;
            out.m00 = cosT * sx;
            out.m10 = sinT * sx;
            out.m01 = cosT * shear - sinT * sy;
            out.m11 = sinT * shear + cosT * sy;
            out.tx  = tx;
            out.ty  = ty;
            return out;
        }

        // Identity/zero value — needed for spring initial velocity
        static FAffineTransform Identity() { return FAffineTransform::Identity(); }

        // Spring arithmetic helpers — element-wise, treating the matrix as a 6-vector
        static FAffineTransform Add(const FAffineTransform& a, const FAffineTransform& b)
        {
            FAffineTransform out;
            out.m00 = a.m00 + b.m00; out.m01 = a.m01 + b.m01;
            out.m10 = a.m10 + b.m10; out.m11 = a.m11 + b.m11;
            out.tx  = a.tx  + b.tx;  out.ty  = a.ty  + b.ty;
            return out;
        }

        static FAffineTransform Scale(const FAffineTransform& a, f32 s)
        {
            FAffineTransform out;
            out.m00 = a.m00 * s; out.m01 = a.m01 * s;
            out.m10 = a.m10 * s; out.m11 = a.m11 * s;
            out.tx  = a.tx  * s; out.ty  = a.ty  * s;
            return out;
        }

        static f32 SquaredMagnitude(const FAffineTransform& v)
        {
            return v.m00*v.m00 + v.m01*v.m01 + v.m10*v.m10 + v.m11*v.m11 + v.tx*v.tx + v.ty*v.ty;
        }
    };

}
