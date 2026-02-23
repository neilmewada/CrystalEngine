#pragma once

namespace CE
{
    // Access properties: left, top, right, bottom
	using FMargin = Vec4;

    struct FUSIONCORE_API FAffineTransform
    {
        float m00 = 1.0f, m01 = 0.0f;
        float m10 = 0.0f, m11 = 1.0f;
        float tx = 0.0f, ty = 0.0f;

        // ---------------------------
        // Constructors / factories
        // ---------------------------

        static consteval FAffineTransform Identity()
        {
            return FAffineTransform{};
        }

        static FAffineTransform Translation(const Vec2& t)
        {
            FAffineTransform out;
            out.tx = t.x;
            out.ty = t.y;
            return out;
        }

        static FAffineTransform Scale(const Vec2& s)
        {
            FAffineTransform out;
            out.m00 = s.x;
            out.m11 = s.y;
            return out;
        }

        // Rotation in radians, counter-clockwise
        static FAffineTransform Rotation(float radians)
        {
            const float c = std::cos(radians);
            const float s = std::sin(radians);

            FAffineTransform out;
            out.m00 = c; out.m01 = -s;
            out.m10 = s; out.m11 = c;
            return out;
        }

        // Shear factors:
        // x' = x + shx*y
        // y' = y + shy*x
        static FAffineTransform Shear(float shx, float shy)
        {
            FAffineTransform out;
            out.m00 = 1.0f; out.m01 = shx;
            out.m10 = shy;  out.m11 = 1.0f;
            return out;
        }

        bool IsTranslationOnly(float epsilon = 1e-8f) const
        {
            return std::fabs(m00 - 1.0f) <= epsilon && std::fabs(m11 - 1.0f) <= epsilon &&
                std::fabs(m01) <= epsilon && std::fabs(m10) <= epsilon;
		}

        float Determinant() const
        {
            return m00 * m11 - m01 * m10;
        }

        bool HasInverse(float epsilon = 1e-8f) const
        {
            return std::fabs(Determinant()) > epsilon;
        }

        bool IsFinite() const
        {
            return std::isfinite(m00) && std::isfinite(m01) &&
                std::isfinite(m10) && std::isfinite(m11) &&
                std::isfinite(tx) && std::isfinite(ty);
        }

        Vec2 TransformPoint(const Vec2& p) const
        {
            // Includes translation
            return Vec2(
                m00 * p.x + m01 * p.y + tx,
                m10 * p.x + m11 * p.y + ty
            );
        }

        Vec2 TransformDirection(const Vec2& v) const
        {
            // Ignores translation (direction)
            return Vec2(
                m00 * v.x + m01 * v.y,
                m10 * v.x + m11 * v.y
            );
        }

        // Composition:
        // (A * B).TransformPoint(p) == A.TransformPoint(B.TransformPoint(p))
        friend FAffineTransform operator*(const FAffineTransform& A, const FAffineTransform& B)
        {
            ZoneScoped;

            FAffineTransform out;

            // Linear part: A.linear * B.linear
            out.m00 = A.m00 * B.m00 + A.m01 * B.m10;
            out.m01 = A.m00 * B.m01 + A.m01 * B.m11;
            out.m10 = A.m10 * B.m00 + A.m11 * B.m10;
            out.m11 = A.m10 * B.m01 + A.m11 * B.m11;

            // Translation: A.linear * B.translation + A.translation
            out.tx = A.m00 * B.tx + A.m01 * B.ty + A.tx;
            out.ty = A.m10 * B.tx + A.m11 * B.ty + A.ty;

            return out;
        }

        inline bool operator==(const FAffineTransform& rhs) const
        {
            return Math::ApproxEquals(m00, rhs.m00) && Math::ApproxEquals(m01, rhs.m01) &&
                Math::ApproxEquals(m10, rhs.m10) && Math::ApproxEquals(m11, rhs.m11) &&
                Math::ApproxEquals(tx, rhs.tx) && Math::ApproxEquals(ty, rhs.ty);
		}

        inline bool operator!=(const FAffineTransform& rhs) const
        {
            return !operator==(rhs);
		}

        FAffineTransform& operator*=(const FAffineTransform& rhs)
        {
            *this = (*this) * rhs;
            return *this;
        }

        // Inverse transform (if invertible)
        FAffineTransform Inverse(float epsilon = 1e-8f) const
        {
            ZoneScoped;

            const float det = Determinant();
            if (std::fabs(det) <= epsilon)
            {
                // Non-invertible: return Identity as a safe fallback.
                // Alternatively: assert/log in your engine.
                return Identity();
            }

            const float invDet = 1.0f / det;

            // invLinear = 1/det * [ m11 -m01; -m10 m00 ]
            const float i00 = m11 * invDet;
            const float i01 = -m01 * invDet;
            const float i10 = -m10 * invDet;
            const float i11 = m00 * invDet;

            // invTranslation = -(invLinear * t)
            const float itx = -(i00 * tx + i01 * ty);
            const float ity = -(i10 * tx + i11 * ty);

            FAffineTransform out;
            out.m00 = i00; out.m01 = i01;
            out.m10 = i10; out.m11 = i11;
            out.tx = itx; out.ty = ity;
            return out;
        }

        // Returns the axis-aligned bounding box of a transformed rect (by transforming 4 corners).
        Rect TransformAABB(const Rect& r) const
        {
            ZoneScoped;

            const Vec2 p0 = TransformPoint(Vec2(r.min.x, r.min.y));
            const Vec2 p1 = TransformPoint(Vec2(r.max.x, r.min.y));
            const Vec2 p2 = TransformPoint(Vec2(r.min.x, r.max.y));
            const Vec2 p3 = TransformPoint(Vec2(r.max.x, r.max.y));

            Vec2 mn(
                std::min(std::min(p0.x, p1.x), std::min(p2.x, p3.x)),
                std::min(std::min(p0.y, p1.y), std::min(p2.y, p3.y))
            );
            Vec2 mx(
                std::max(std::max(p0.x, p1.x), std::max(p2.x, p3.x)),
                std::max(std::max(p0.y, p1.y), std::max(p2.y, p3.y))
            );

            return Rect{ mn, mx };
        }

        void SerializePOD(Stream* stream)
        {
			*stream << m00 << m01 << m10 << m11 << tx << ty;
        }

        void DeserializePOD(Stream* stream)
        {
			*stream >> m00 >> m01 >> m10 >> m11 >> tx >> ty;
        }
    };
    
} // namespace CE

CE_RTTI_POD(FUSIONCORE_API, CE, FAffineTransform);
