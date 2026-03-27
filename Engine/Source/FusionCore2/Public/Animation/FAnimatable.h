#pragma once

namespace CE
{

    template<typename T>
    struct FAnimatable
    {
        static constexpr bool Supported = requires (T a, T b) { a + b; };

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
        static constexpr bool Supported = true;

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
        static constexpr bool Supported = true;

        static FAffineTransform Lerp(const FAffineTransform& a, const FAffineTransform& b, f32 t)
        {
            return LerpUnclamped(a, b, Math::Clamp01(t));
        }

        static FAffineTransform LerpUnclamped(const FAffineTransform& a, const FAffineTransform& b, f32 t)
        {
            ZoneScoped;

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

    template<>
    struct FAnimatable<FPen>
    {
        static constexpr bool Supported = true;

        static FPen Lerp(const FPen& a, const FPen& b, f32 t)
        {
            return LerpUnclamped(a, b, Math::Clamp01(t));
        }

        static FPen LerpUnclamped(const FPen& a, const FPen& b, f32 t)
        {
            ZoneScoped;

            // Scalar fields: lerp continuously regardless of style
            const f32 thickness       = a.GetThickness()       + (b.GetThickness()       - a.GetThickness())       * t;
            const f32 dashLength      = a.GetDashLength()      + (b.GetDashLength()      - a.GetDashLength())      * t;
            const f32 dashGap         = a.GetDashGap()         + (b.GetDashGap()         - a.GetDashGap())         * t;
            const f32 gradientOffset  = a.GetGradientOffset()  + (b.GetGradientOffset()  - a.GetGradientOffset())  * t;

            // Style (Solid/Dashed/Dotted/None): discrete, snap at t=0.5 if different
            const FPenStyle style = (a.GetStyle() == b.GetStyle() || t < 0.5f) ? a.GetStyle() : b.GetStyle();

            // Color / gradient fill: mirror FBrush SolidFill<->Gradient logic
            FPen out = LerpColorFill(a, b, t);
            out.SetStyle(style);
            out.SetThickness(thickness);
            out.SetDashLength(dashLength);
            out.SetDashGap(dashGap);
            out.SetGradientOffset(gradientOffset);
            return out;
        }

        // Identity/zero value — needed for spring initial velocity
        static FPen Identity() { return FPen(); }

        // Spring arithmetic helpers
        static FPen Add(const FPen& a, const FPen& b)
        {
            ZoneScoped;

            // None acts as additive zero: adopt the other side's structure
            if (a.GetStyle() == FPenStyle::None && b.GetStyle() != FPenStyle::None)
            {
                FPen out = b;
                out.SetColor(FAnimatable<Color>::Add(a.GetColor(), b.GetColor()));
                out.SetThickness(a.GetThickness() + b.GetThickness());
                out.SetDashLength(a.GetDashLength() + b.GetDashLength());
                out.SetDashGap(a.GetDashGap() + b.GetDashGap());
                out.SetGradientOffset(a.GetGradientOffset() + b.GetGradientOffset());
                return out;
            }

            // Build on a's structure
            FPen out = a;
            out.SetThickness(a.GetThickness() + b.GetThickness());
            out.SetDashLength(a.GetDashLength() + b.GetDashLength());
            out.SetDashGap(a.GetDashGap() + b.GetDashGap());
            out.SetGradientOffset(a.GetGradientOffset() + b.GetGradientOffset());
            out.SetColor(FAnimatable<Color>::Add(a.GetColor(), b.GetColor()));

            if (a.HasGradient() && b.HasGradient())
            {
                const FGradient& ga = a.GetGradient();
                const FGradient& gb = b.GetGradient();
                if (ga.stops.GetSize() == gb.stops.GetSize())
                {
                    FGradient outGrad = ga;
                    for (int i = 0; i < (int)ga.stops.GetSize(); i++)
                    {
                        outGrad.stops[i].color    = FAnimatable<Color>::Add(ga.stops[i].color, gb.stops[i].color);
                        outGrad.stops[i].position = ga.stops[i].position + gb.stops[i].position;
                    }
                    outGrad.angle = ga.angle + gb.angle;
                    out.SetGradient(outGrad);
                }
            }

            return out;
        }

        static FPen Scale(const FPen& a, f32 s)
        {
            ZoneScoped;

            FPen out = a;
            out.SetThickness(a.GetThickness() * s);
            out.SetDashLength(a.GetDashLength() * s);
            out.SetDashGap(a.GetDashGap() * s);
            out.SetGradientOffset(a.GetGradientOffset() * s);
            out.SetColor(FAnimatable<Color>::Scale(a.GetColor(), s));

            if (a.HasGradient())
            {
                const FGradient& g = a.GetGradient();
                FGradient outGrad = g;
                for (int i = 0; i < (int)g.stops.GetSize(); i++)
                {
                    outGrad.stops[i].color    = FAnimatable<Color>::Scale(g.stops[i].color, s);
                    outGrad.stops[i].position = g.stops[i].position * s;
                }
                outGrad.angle = g.angle * s;
                out.SetGradient(outGrad);
            }

            return out;
        }

        static f32 SquaredMagnitude(const FPen& v)
        {
            ZoneScoped;

            f32 mag = FAnimatable<Color>::SquaredMagnitude(v.GetColor())
                    + v.GetThickness()      * v.GetThickness()
                    + v.GetDashLength()     * v.GetDashLength()
                    + v.GetDashGap()        * v.GetDashGap()
                    + v.GetGradientOffset() * v.GetGradientOffset();

            if (v.HasGradient())
            {
                const FGradient& g = v.GetGradient();
                for (int i = 0; i < (int)g.stops.GetSize(); i++)
                    mag += FAnimatable<Color>::SquaredMagnitude(g.stops[i].color);
                mag += g.angle * g.angle;
            }

            return mag;
        }

    private:

        // Interpolate only the color/gradient fill; thickness/dash/style set separately by LerpUnclamped
        static FPen LerpColorFill(const FPen& a, const FPen& b, f32 t)
        {
            ZoneScoped;

            const bool aHasGrad = a.HasGradient();
            const bool bHasGrad = b.HasGradient();

            if (aHasGrad && bHasGrad)
                return LerpGradientPen(a, b, t);

            if (!aHasGrad && !bHasGrad)
            {
                FPen out = a;
                out.SetColor(Color::Lerp(a.GetColor(), b.GetColor(), t));
                return out;
            }

            // Solid color <-> Gradient: treat solid as a degenerate gradient
            if (!aHasGrad && bHasGrad)
                return LerpSolidGradientPen(a, b, t);         // t=0 → solid a, t=1 → gradient b
            else
                return LerpSolidGradientPen(b, a, 1.0f - t); // t=0 → gradient a, t=1 → solid b
        }

        // Both pens have gradients
        static FPen LerpGradientPen(const FPen& a, const FPen& b, f32 t)
        {
            ZoneScoped;

            const FGradient& ga = a.GetGradient();
            const FGradient& gb = b.GetGradient();

            // Snap at t=0.5 if gradient type or stop count differ
            if (ga.gradientType != gb.gradientType || ga.stops.GetSize() != gb.stops.GetSize())
                return t < 0.5f ? a : b;

            FGradient outGrad = ga;
            for (int i = 0; i < (int)ga.stops.GetSize(); i++)
            {
                outGrad.stops[i].color    = Color::Lerp(ga.stops[i].color, gb.stops[i].color, t);
                outGrad.stops[i].position = ga.stops[i].position + (gb.stops[i].position - ga.stops[i].position) * t;
            }
            outGrad.angle = ga.angle + (gb.angle - ga.angle) * t;

            // Thickness is a placeholder; LerpUnclamped overwrites it
            return FPen(outGrad, 1.0f, Color::Lerp(a.GetColor(), b.GetColor(), t));
        }

        // solid: solid-color pen, gradient: gradient pen; t goes 0→1 solid→gradient
        static FPen LerpSolidGradientPen(const FPen& solid, const FPen& gradient, f32 t)
        {
            ZoneScoped;

            const FGradient& g = gradient.GetGradient();

            FGradient outGrad = g;
            for (int i = 0; i < (int)g.stops.GetSize(); i++)
                outGrad.stops[i].color = Color::Lerp(solid.GetColor(), g.stops[i].color, t);
            outGrad.angle = g.angle * t;

            // Tint lerps from White (no tint at t=0) to the gradient pen's tint
            return FPen(outGrad, 1.0f, Color::Lerp(Colors::White, gradient.GetColor(), t));
        }
    };

    template<>
    struct FAnimatable<FBrush>
    {
        static constexpr bool Supported = true;

        static FBrush Lerp(const FBrush& a, const FBrush& b, f32 t)
        {
            return LerpUnclamped(a, b, Math::Clamp01(t));
        }

        static FBrush LerpUnclamped(const FBrush& a, const FBrush& b, f32 t)
        {
            ZoneScoped;

            const FBrushStyle styleA = a.GetBrushStyle();
            const FBrushStyle styleB = b.GetBrushStyle();

            if (styleA == styleB)
            {
                switch (styleA)
                {
                case FBrushStyle::SolidFill: return LerpSolidFill(a, b, t);
                case FBrushStyle::Gradient:  return LerpGradient(a, b, t);
                case FBrushStyle::Image:     return LerpImage(a, b, t);
                default:                     return t < 1.0f ? a : b;
                }
            }

            // SolidFill <-> Gradient: treat solid as a degenerate gradient
            if (styleA == FBrushStyle::SolidFill && styleB == FBrushStyle::Gradient)
                return LerpSolidGradient(a, b, t);
            if (styleA == FBrushStyle::Gradient && styleB == FBrushStyle::SolidFill)
                return LerpSolidGradient(b, a, 1.0f - t);

            // All other cross-style mismatches: snap at t=0.5
            return t < 0.5f ? a : b;
        }

        static FBrush Identity() { return FBrush(); }

        // Spring arithmetic helpers
        static FBrush Add(const FBrush& a, const FBrush& b)
        {
            ZoneScoped;

            const FBrushStyle styleA = a.GetBrushStyle();
            const FBrushStyle styleB = b.GetBrushStyle();

            // None acts as a zero element: adopt the other side's structure
            if (styleA == FBrushStyle::None && styleB != FBrushStyle::None)
            {
                FBrush out = b;
                out.SetColor(FAnimatable<Color>::Add(a.GetColor(), b.GetColor()));
                return out;
            }

            if (styleA == FBrushStyle::Gradient && styleA == styleB)
            {
                const FGradient& ga = a.GetGradient();
                const FGradient& gb = b.GetGradient();
                if (ga.stops.GetSize() == gb.stops.GetSize())
                {
                    FGradient outGrad = ga;
                    for (int i = 0; i < (int)ga.stops.GetSize(); i++)
                    {
                        outGrad.stops[i].color    = FAnimatable<Color>::Add(ga.stops[i].color, gb.stops[i].color);
                        outGrad.stops[i].position = ga.stops[i].position + gb.stops[i].position;
                    }
                    outGrad.angle = ga.angle + gb.angle;
                    return FBrush(outGrad, FAnimatable<Color>::Add(a.GetColor(), b.GetColor()));
                }
            }

            if (styleA == FBrushStyle::Image && styleA == styleB)
            {
                FBrush out = a;
                out.SetColor(FAnimatable<Color>::Add(a.GetColor(), b.GetColor()));
                out.SetBrushSize(a.GetBrushSize() + b.GetBrushSize());
                out.SetBrushPosition(a.GetBrushPosition() + b.GetBrushPosition());
                return out;
            }

            // SolidFill and None (zero velocity) styles: add color only
            FBrush out = a;
            out.SetColor(FAnimatable<Color>::Add(a.GetColor(), b.GetColor()));
            return out;
        }

        static FBrush Scale(const FBrush& a, f32 s)
        {
            ZoneScoped;

            const FBrushStyle style = a.GetBrushStyle();

            if (style == FBrushStyle::Gradient)
            {
                const FGradient& g = a.GetGradient();
                FGradient outGrad = g;
                for (int i = 0; i < (int)g.stops.GetSize(); i++)
                {
                    outGrad.stops[i].color    = FAnimatable<Color>::Scale(g.stops[i].color, s);
                    outGrad.stops[i].position = g.stops[i].position * s;
                }
                outGrad.angle = g.angle * s;
                return FBrush(outGrad, FAnimatable<Color>::Scale(a.GetColor(), s));
            }

            if (style == FBrushStyle::Image)
            {
                FBrush out = a;
                out.SetColor(FAnimatable<Color>::Scale(a.GetColor(), s));
                out.SetBrushSize(a.GetBrushSize() * s);
                out.SetBrushPosition(a.GetBrushPosition() * s);
                return out;
            }

            // SolidFill and None: scale color only
            FBrush out = a;
            out.SetColor(FAnimatable<Color>::Scale(a.GetColor(), s));
            return out;
        }

        static f32 SquaredMagnitude(const FBrush& v)
        {
            ZoneScoped;

            f32 mag = FAnimatable<Color>::SquaredMagnitude(v.GetColor());

            if (v.GetBrushStyle() == FBrushStyle::Gradient)
            {
                const FGradient& g = v.GetGradient();
                for (int i = 0; i < (int)g.stops.GetSize(); i++)
                    mag += FAnimatable<Color>::SquaredMagnitude(g.stops[i].color);
                mag += g.angle * g.angle;
            }

            return mag;
        }

    private:

        static FBrush LerpSolidFill(const FBrush& a, const FBrush& b, f32 t)
        {
            ZoneScoped;

            return FBrush(Color::Lerp(a.GetColor(), b.GetColor(), t), FBrushStyle::SolidFill);
        }

        static FBrush LerpGradient(const FBrush& a, const FBrush& b, f32 t)
        {
            ZoneScoped;

            const FGradient& ga = a.GetGradient();
            const FGradient& gb = b.GetGradient();

            // Snap at t=0.5 if gradient type or stop count differ
            if (ga.gradientType != gb.gradientType || ga.stops.GetSize() != gb.stops.GetSize())
                return t < 0.5f ? a : b;

            FGradient outGrad = ga;
            for (int i = 0; i < (int)ga.stops.GetSize(); i++)
            {
                outGrad.stops[i].color    = Color::Lerp(ga.stops[i].color, gb.stops[i].color, t);
                outGrad.stops[i].position = ga.stops[i].position + (gb.stops[i].position - ga.stops[i].position) * t;
            }
            outGrad.angle = ga.angle + (gb.angle - ga.angle) * t;

            return FBrush(outGrad, Color::Lerp(a.GetColor(), b.GetColor(), t));
        }

        static FBrush LerpImage(const FBrush& a, const FBrush& b, f32 t)
        {
            ZoneScoped;

            const Name& imgA = a.GetImageName();
            const Name& imgB = b.GetImageName();

            // Snap image name at t=0.5 if different; discrete fields snap at t=1
            const Name& outImg = (imgA == imgB || t < 0.5f) ? imgA : imgB;
            FBrush out(outImg, Color::Lerp(a.GetColor(), b.GetColor(), t));
            out.SetImageFit(t < 1.0f ? a.GetImageFit() : b.GetImageFit());
            out.SetBrushTiling(t < 1.0f ? a.GetBrushTiling() : b.GetBrushTiling());
            out.SetSliceMargins(t < 1.0f ? a.GetSliceMargins() : b.GetSliceMargins());
            out.SetBrushSize(a.GetBrushSize() + (b.GetBrushSize() - a.GetBrushSize()) * t);
            out.SetBrushPosition(a.GetBrushPosition() + (b.GetBrushPosition() - a.GetBrushPosition()) * t);
            return out;
        }

        // solid is the SolidFill brush, gradient is the Gradient brush, t goes 0->1 solid->gradient
        static FBrush LerpSolidGradient(const FBrush& solid, const FBrush& gradient, f32 t)
        {
            ZoneScoped;

            const FGradient& g = gradient.GetGradient();

            FGradient outGrad = g;
            for (int i = 0; i < (int)g.stops.GetSize(); i++)
                outGrad.stops[i].color = Color::Lerp(solid.GetColor(), g.stops[i].color, t);
            outGrad.angle = g.angle * t;

            // Tint lerps from white (no tint at t=0) to the gradient's tint
            return FBrush(outGrad, Color::Lerp(Colors::White, gradient.GetColor(), t));
        }
    };

}
