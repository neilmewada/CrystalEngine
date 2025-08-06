#pragma once


namespace CE
{
    template<typename T>
    class TVector2;

    template<typename T>
    class TVector3;

    template<typename T>
    class TVector4;

    typedef TVector2<f32> Vec2;
    typedef TVector3<f32> Vec3;
    typedef TVector4<f32> Vec4;

    typedef TVector2<s32> Vec2i;
    typedef TVector3<s32> Vec3i;
    typedef TVector4<s32> Vec4i;

	typedef Vec2 Range;
	typedef Vec2i RangeInt;

    inline bool ApproxEquals(f32 lhs, f32 rhs)
    {
        return std::abs(lhs - rhs) < std::numeric_limits<f32>::epsilon();
    }

    template<typename T>
    class TVector2
    {
    public:

        constexpr TVector2() : x(0), y(0)
        {}

        constexpr TVector2(T x, T y) : x(x), y(y)
        {}

        TVector2(TVector3<T> vec3);
        TVector2(TVector4<T> vec4);

        union {
            struct {
                T x, y;
            };
			struct {
				T min, max;
			};
			struct {
				T left, right;
			};
			struct {
				T top, bottom;
			};
			struct {
				T width, height;
			};
            T xy[2];
        };

		FORCE_INLINE SIZE_T GetHash() const
		{
			return GetCombinedHashes({
                CE::GetHash(x), CE::GetHash(y)
            });
		}

        constexpr Vec2 ToVec2() const
		{
            return Vec2(x, y);
		}

        TVector2<s32> ToVec2i() const
        {
            return TVector2<s32>((s32)x, (s32)y);
        }

        constexpr  TVector2 operator+(const TVector2& rhs) const
        {
            return TVector2(x + rhs.x, y + rhs.y);
        }

        constexpr  TVector2 operator-(const TVector2& rhs) const
        {
            return TVector2(x - rhs.x, y - rhs.y);
        }

        constexpr  TVector2 operator+() const
        {
            return *this;
        }

        constexpr  TVector2 operator-() const
        {
            return TVector2(-x, -y);
        }

        constexpr  TVector2 operator+=(const TVector2& rhs)
        {
            *this = *this + rhs;
            return *this;
        }

        constexpr  TVector2 operator-=(const TVector2& rhs)
        {
            *this = *this - rhs;
            return *this;
        }

        constexpr  bool operator==(const TVector2& rhs) const
        {
            return Math::ApproxEquals(x, rhs.x) && Math::ApproxEquals(y, rhs.y);
        }

        constexpr  bool operator!=(const TVector2& rhs) const
        {
            return !operator==(rhs);
        }

        constexpr  TVector2 operator*(s32 value) const
        {
            return TVector2(value * x, value * y);
        }

        constexpr  TVector2 operator*(u32 value) const
        {
            return TVector2(value * x, value * y);
        }

        constexpr  TVector2 operator*(f32 value) const
        {
            return TVector2(value * x, value * y);
        }

        constexpr  TVector2 operator*(TVector2 value) const
        {
            return TVector2(value.x * x, value.y * y);
        }

        constexpr  TVector2 operator*=(s32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr  TVector2 operator*=(u32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr  TVector2 operator*=(f32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr  TVector2 operator*=(TVector2 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr  TVector2 operator/(f32 value) const
        {
            return TVector2(x / value, y / value);
        }

        constexpr  TVector2 operator/=(f32 value)
        {
            *this = *this / value;
            return *this;
        }

        constexpr  f32 GetSqrMagnitude() const
        {
            return x * x + y * y;
        }

        inline f32 GetMagnitude() const
        {
            return sqrt(GetSqrMagnitude());
        }

        inline TVector2 GetNormalized() const
        {
            return *this / GetMagnitude();
        }

        f32 GetMax() const
        {
            return Math::Max({x, y});
        }

        constexpr static f32 SqrDistance(TVector2 a, TVector2 b)
		{
            return (b - a).GetSqrMagnitude();
		}

        static f32 Distance(TVector2 a, TVector2 b)
		{
            return (b - a).GetMagnitude();
		}

        constexpr static T Dot(TVector2 a, TVector2 b)
        {
            return a.x * b.x + a.y * b.y;
        }

        constexpr f32 Dot(TVector2 b) const
        {
            return x * b.x + y * b.y;
        }

        /// Signed angle in radians between 2 vectors
        inline static f32 SignedAngle(TVector2 a, TVector2 b)
        {
            return Math::ACos(Dot(a, b) / (a.GetMagnitude() * b.GetMagnitude())); // TODO
        }

        /// Signed angle in radians between 2 vectors
        inline f32 SignedAngle(TVector2 b) const
        {
            return SignedAngle(*this, b);
        }

        inline static TVector2<f32> Lerp(TVector2 from, TVector2 to, f32 t)
        {
            return TVector2<f32>(Math::Lerp(from.x, to.x, t), Math::Lerp(from.y, to.y, t));
        }

        inline static TVector2<f32> LerpUnclamped(TVector2 from, TVector2 to, f32 t)
        {
            return TVector2<f32>(Math::LerpUnclamped(from.x, to.x, t), Math::LerpUnclamped(from.y, to.y, t));
        }

        inline String ToString() const
        {
            return String::Format("({}, {})", x, y);
        }
    };
    
    template<typename T>
    class TVector3
    {
    public:

        constexpr TVector3() : x(0), y(0), z(0)
        {}

        constexpr TVector3(T x, T y) : x(x), y(y), z(0)
        {}

        constexpr TVector3(T x, T y, T z) : x(x), y(y), z(z)
        {}

        constexpr TVector3(TVector2<T> vec2);
        constexpr TVector3(TVector4<T> vec4);

        union {
            struct {
                T x, y, z;
            };
            T xyz[4]; // size/alignment is same as Vector4
        };

		FORCE_INLINE SIZE_T GetHash() const
		{
			return GetCombinedHashes({
                CE::GetHash(x), CE::GetHash(y), CE::GetHash(z)
            });
		}

        inline T operator[](int index) const
        {
            return xyz[index];
        }

        constexpr Vec3 ToVec3() const
        {
            return Vec3(x, y, z);
        }

        constexpr TVector3<s32> ToVec3i() const
        {
            return TVector3<s32>((s32)x, (s32)y, (s32)z);
        }

        constexpr TVector3 operator+(const TVector3& rhs) const
        {
            return TVector3(x + rhs.x, y + rhs.y, z + rhs.z);
        }

        constexpr TVector3 operator-(const TVector3& rhs) const
        {
            return TVector3(x - rhs.x, y - rhs.y, z - rhs.z);
        }

        constexpr TVector3 operator+() const
        {
            return *this;
        }

        constexpr TVector3 operator-() const
        {
            return TVector3(-x, -y, -z);
        }

        constexpr TVector3 operator+=(const TVector3& rhs)
        {
            *this = *this + rhs;
            return *this;
        }

        constexpr TVector3 operator-=(const TVector3& rhs)
        {
            *this = *this - rhs;
            return *this;
        }

        constexpr bool operator==(const TVector3& rhs) const
        {
            return Math::ApproxEquals(x, rhs.x) && Math::ApproxEquals(y, rhs.y) && Math::ApproxEquals(z, rhs.z);
        }

        constexpr bool operator!=(const TVector3& rhs) const
        {
            return !operator==(rhs);
        }

        constexpr TVector3 operator*(s32 value) const
        {
            return TVector3(value * x, value * y, value * z);
        }

        constexpr TVector3 operator*(u32 value) const
        {
            return TVector3(value * x, value * y, value * z);
        }

        constexpr TVector3 operator*(f32 value) const
        {
            return TVector3(value * x, value * y, value * z);
        }

        constexpr TVector3 operator*(const TVector3& value) const
        {
            return TVector3(value.x * x, value.y * y, value.z * z);
        }

        constexpr TVector3 operator*=(s32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr TVector3 operator*=(u32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr TVector3 operator*=(f32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr TVector3 operator*=(const TVector3& value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr TVector3 operator/(f32 value) const
        {
            return TVector3(x / value, y / value, z / value);
        }

        constexpr TVector3 operator/=(f32 value)
        {
            *this = *this / value;
            return *this;
        }

        constexpr f32 GetSqrMagnitude() const
        {
            return x * x + y * y + z * z;
        }

        inline f32 GetMagnitude() const
        {
            return sqrt(GetSqrMagnitude());
        }

        inline TVector3 GetNormalized() const
        {
            return *this / GetMagnitude();
        }

        constexpr f32 GetMax() const
        {
            return Math::Max({x, y, z});
        }

        constexpr static T Dot(TVector3 a, TVector3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        constexpr f32 Dot(TVector3 b) const
        {
            return x * b.x + y * b.y + z * b.z;
        }

        constexpr static TVector3 Cross(TVector3 a, TVector3 b)
        {
            return TVector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
        }

        constexpr TVector3 Cross(TVector3 b) const
        {
            return TVector3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
        }

        /// Signed angle in radians between 2 vectors
        inline static f32 SignedAngle(TVector3 a, TVector3 b)
        {
            return Math::ACos(Dot(a, b) / (a.GetMagnitude() * b.GetMagnitude())); // TODO
        }

        /// Signed angle in radians between 2 vectors
        inline f32 SignedAngle(TVector3 b) const
        {
            return SignedAngle(*this, b);
        }

        constexpr static f32 SqrDistance(TVector3 a, TVector3 b)
        {
            return (b - a).GetSqrMagnitude();
        }

        static f32 Distance(TVector3 a, TVector3 b)
        {
            return (b - a).GetMagnitude();
        }

        inline static TVector3<f32> Lerp(TVector3 from, TVector3 to, f32 t)
        {
            return TVector3<f32>(Math::Lerp(from.x, to.x, t), Math::Lerp(from.y, to.y, t), Math::Lerp(from.z, to.z, t));
        }

        inline static TVector3<f32> LerpUnclamped(TVector3 from, TVector3 to, f32 t)
        {
            return TVector3<f32>(Math::LerpUnclamped(from.x, to.x, t), Math::LerpUnclamped(from.y, to.y, t), Math::LerpUnclamped(from.z, to.z, t));
        }

        inline String ToString() const
        {
            return String::Format("({}, {}, {})", x, y, z);
        }
    };

    template<typename T>
    class TVector4
    {
    public:

        constexpr TVector4() : x(0), y(0), z(0), w(0)
        {}

        constexpr TVector4(T x, T y) : x(x), y(y), z(0), w(0)
        {}

        constexpr TVector4(T x, T y, T z) : x(x), y(y), z(z), w(0)
        {}

        constexpr TVector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w)
		{}

        TVector4(TVector2<T> vec2);
		TVector4(TVector2<T> xy, TVector2<T> zw);
        TVector4(TVector3<T> vec3);
        
        TVector4(TVector3<T> vec3, T w) : x(vec3.x), y(vec3.y), z(vec3.z), w(w)
        {}

        union {
            struct {
                T x, y, z, w;
            };
			struct {
				T left, top, right, bottom;
			};
			struct {
				T topLeft, topRight, bottomRight, bottomLeft;
			};
			struct {
				TVector2<T> min, max;
			};
			struct {
				TVector2<T> pos, size;
			};
            T xyzw[4];
        };

		FORCE_INLINE SIZE_T GetHash() const
		{
			return GetCombinedHashes({
                CE::GetHash(x), CE::GetHash(y), CE::GetHash(z), CE::GetHash(w)
            });
		}

        Vec4 ToVec4() const
        {
            return Vec4(x, y, z, w);
        }

        TVector4<s32> ToVec4i() const
        {
            return TVector4<s32>((s32)x, (s32)y, (s32)z, (s32)z);
        }

        inline T& operator[](int index)
        {
            return xyzw[index];
        }

        inline const T& operator[](int index) const
        {
            return xyzw[index];
        }

        constexpr TVector4 operator+(const TVector4& rhs) const
        {
            return TVector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
        }

        constexpr TVector4 operator-(const TVector4& rhs) const
        {
            return TVector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
        }

        constexpr TVector4 operator+() const
        {
            return *this;
        }

        constexpr TVector4 operator-() const
        {
            return TVector4(-x, -y, -z, -w);
        }

        constexpr TVector4 operator+=(const TVector4& rhs)
        {
            *this = *this + rhs;
            return *this;
        }

        constexpr TVector4 operator-=(const TVector4& rhs)
        {
            *this = *this - rhs;
            return *this;
        }

        constexpr bool operator==(const TVector4& rhs) const
        {
            return Math::ApproxEquals(x, rhs.x) && Math::ApproxEquals(y, rhs.y) && Math::ApproxEquals(z, rhs.z) &&
                Math::ApproxEquals(w, rhs.w);
        }

        constexpr bool operator!=(const TVector4& rhs) const
        {
            return !operator==(rhs);
        }

        constexpr TVector4 operator*(s32 value) const
        {
            return TVector4(value * x, value * y, value * z, value * w);
        }

        constexpr TVector4 operator*(u32 value) const
        {
            return TVector4(value * x, value * y, value * z, value * w);
        }

        constexpr TVector4 operator*(f32 value) const
        {
            return TVector4(value * x, value * y, value * z, value * w);
        }

        constexpr TVector4 operator*(const TVector4& value) const
        {
            return TVector4(value.x * x, value.y * y, value.z * z, value.w * w);
        }

        constexpr TVector4 operator*=(s32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr TVector4 operator*=(u32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr TVector4 operator*=(f32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr TVector4 operator*=(const TVector4& value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr TVector4 operator/(f32 value) const
        {
            return TVector4(x / value, y / value, z / value, w / value);
        }

        constexpr TVector4 operator/=(f32 value)
        {
            *this = *this / value;
            return *this;
        }

        constexpr f32 GetSqrMagnitude() const
        {
            return x * x + y * y + z * z + w * w;
        }

        inline f32 GetMagnitude() const
        {
            return sqrt(GetSqrMagnitude());
        }

        inline TVector4 GetNormalized() const
        {
            return *this / GetMagnitude();
        }

        f32 GetMax() const
		{
            return Math::Max({x, y, z, w});
		}

        constexpr static f32 Dot(TVector4 a, TVector4 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        }

        constexpr f32 Dot(TVector4 b) const
        {
            return x * b.x + y * b.y + z * b.z + w * b.w;
        }

        constexpr static TVector4 Cross(TVector4 a, TVector4 b)
        {
            return TVector4(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, a.w * b.w);
        }

        constexpr TVector4 Cross(TVector4 b) const
        {
            return TVector4(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x, w * b.w);
        }

        constexpr static f32 SqrDistance(TVector4 a, TVector4 b)
		{
            return (b - a).GetSqrMagnitude();
		}

        static f32 Distance(TVector4 a, TVector4 b)
		{
            return (b - a).GetMagnitude();
		}

        /// Signed angle in radians between 2 vectors
        inline static f32 SignedAngle(TVector4 a, TVector4 b)
        {
            return Math::ACos(Dot(a, b) / (a.GetMagnitude() * b.GetMagnitude())); // TODO
        }

        /// Signed angle in radians between 2 vectors
        inline f32 SignedAngle(TVector4 b) const
        {
            return SignedAngle(*this, b);
        }

        inline static TVector4<f32> Lerp(TVector4 from, TVector4 to, f32 t)
        {
            return TVector4<f32>(Math::Lerp(from.x, to.x, t), Math::Lerp(from.y, to.y, t), Math::Lerp(from.z, to.z, t), Math::Lerp(from.w, to.w, t));
        }

        inline static TVector4<f32> LerpUnclamped(TVector4 from, TVector4 to, f32 t)
        {
            return TVector4<f32>(Math::LerpUnclamped(from.x, to.x, t), Math::LerpUnclamped(from.y, to.y, t), Math::LerpUnclamped(from.z, to.z, t), Math::LerpUnclamped(from.w, to.w, t));
        }

        inline String ToString() const
        {
            return String::Format("({}, {}, {}, {})", x, y, z, w);
        }
    };

    class CORE_API Rect
    {
    public:

        Rect() : left(0), top(0), right(0), bottom(0)
        {}

        Rect(const Vec4& vec) : left(vec.left), top(vec.top), right(vec.right), bottom(vec.bottom)
        {}

        Rect(const Vec2& min, const Vec2& max) : min(min), max(max)
        {}

        Rect(f32 left, f32 top, f32 right, f32 bottom) : left(left), top(top), right(right), bottom(bottom)
        {}

        static Rect FromSize(const Vec2& min, const Vec2& size)
        {
            return Rect(min, min + size);
        }

        static Rect FromSize(f32 x, f32 y, f32 w, f32 h)
        {
            return Rect(x, y, x + w, y + h);
        }

        inline bool Contains(Vec2 point) const
        {
            return point.x >= min.x && point.y >= min.y && point.x <= max.x && point.y <= max.y;
        }

        bool Overlaps(const Rect& other) const
        {
            return !(right < other.left || left > other.right || bottom < other.top || top > other.bottom);
        }

        union {
            struct {
                f32 left, top, right, bottom;
            };
            struct {
                Vec2 min, max;
            };
        };

        inline Vec2 GetSize() const
        {
            return max - min;
        }

        f32 GetArea() const
        {
            return GetSize().width * GetSize().height;
        }

        s32 GetAreaInt() const
        {
            return Math::RoundToInt(GetSize().width * GetSize().height);
        }

        inline Vec4 ToVec4() const
        {
            return Vec4(left, top, right, bottom);
        }

        Rect Scale(Vec2 scale) const
        {
            Vec2 size = GetSize();
            Vec2 center = (min + max) * 0.5f;
            return Rect(center - size / 2.0f * scale, center + size / 2.0f * scale);
        }

        Rect Scale(f32 scale) const
        {
            return Scale(Vec2(scale, scale));
        }

        Rect Translate(Vec2 translation) const
        {
            Vec2 size = GetSize();
            return Rect::FromSize(min + translation, size);
        }

        Rect Encapsulate(const Vec2& point) const
        {
            auto xmin = Math::Min({ min.x, max.x, point.x });
            auto xmax = Math::Max({ min.x, max.x, point.x });
            auto ymin = Math::Min({ min.y, max.y, point.y });
            auto ymax = Math::Max({ min.y, max.y, point.y });
            return Rect(xmin, ymin, xmax, ymax);
        }

        Rect Encapsulate(const Rect& other) const
        {
			auto xmin = Math::Min({ min.x, max.x, other.min.x, other.max.x });
            auto xmax = Math::Max({ min.x, max.x, other.min.x, other.max.x });
            auto ymin = Math::Min({ min.y, max.y, other.min.y, other.max.y });
            auto ymax = Math::Max({ min.y, max.y, other.min.y, other.max.y });
            return Rect(xmin, ymin, xmax, ymax);
        }

        static void RotatePoint(float x, float y, float centerX, float centerY, double angle, float& newX, float& newY) {
            float radians = angle * (M_PI / 180.0); // Convert degrees to radians
            float cosine = std::cos(radians);
            float sine = std::sin(radians);

            newX = centerX + (x - centerX) * cosine - (y - centerY) * sine;
            newY = centerY + (x - centerX) * sine + (y - centerY) * cosine;
        }

        // Function to find the bounding box of a rotated rectangle
        static Rect ComputeBoundingBox(const Rect& rectangle, float angle) {
            float x1 = rectangle.min.x;
            float y1 = rectangle.min.y;
            float x2 = rectangle.max.x;
            float y2 = rectangle.min.y;
            float x3 = rectangle.max.x;
            float y3 = rectangle.max.y;
            float x4 = rectangle.min.x;
            float y4 = rectangle.max.y;

            float centerX = (rectangle.min.x + rectangle.max.x) / 2.0f;
            float centerY = (rectangle.min.y + rectangle.max.y) / 2.0f;

            float newX1, newY1, newX2, newY2, newX3, newY3, newX4, newY4;

            RotatePoint(x1, y1, centerX, centerY, angle, newX1, newY1);
            RotatePoint(x2, y2, centerX, centerY, angle, newX2, newY2);
            RotatePoint(x3, y3, centerX, centerY, angle, newX3, newY3);
            RotatePoint(x4, y4, centerX, centerY, angle, newX4, newY4);

            float minX = std::min({ newX1, newX2, newX3, newX4 });
            float maxX = std::max({ newX1, newX2, newX3, newX4 });
            float minY = std::min({ newY1, newY2, newY3, newY4 });
            float maxY = std::max({ newY1, newY2, newY3, newY4 });

            Rect boundingBox = { minX, minY, maxX - minX, maxY - minY };
            return boundingBox;
        }

        inline bool operator==(const Rect& rhs) const
        {
            return min == rhs.min && max == rhs.max;
        }

        inline bool operator!=(const Rect& rhs) const
        {
            return !operator==(rhs);
        }

        inline Rect operator+(const Rect& rhs)
        {
            return Rect(left + rhs.left, top + rhs.top, right + rhs.right, bottom + rhs.bottom);
        }

        inline Rect operator-(const Rect& rhs)
        {
            return Rect(left - rhs.left, top - rhs.top, right - rhs.right, bottom - rhs.bottom);
        }

        inline Rect& operator+=(const Rect& rhs)
        {
            left += rhs.left; top += rhs.top; right += rhs.right; bottom += rhs.bottom;
            return *this;
        }

        inline Rect& operator-=(const Rect& rhs)
        {
            left -= rhs.left; top -= rhs.top; right -= rhs.right; bottom -= rhs.bottom;
            return *this;
        }

        inline SIZE_T GetHash() const
        {
            SIZE_T hash = CE::GetHash(left);
            CombineHash(hash, top);
            CombineHash(hash, right);
            CombineHash(hash, bottom);
            return hash;
        }

        inline String ToString() const
        {
            return String::Format("({}, {}, {}, {})", left, top, right, bottom);
        }
        
    };

    template<typename T>
    TVector2<T>::TVector2(TVector3<T> vec3) : x(vec3.x), y(vec3.y)
    {}

    template<typename T>
    TVector2<T>::TVector2(TVector4<T> vec4) : x(vec4.x), y(vec4.y)
    {}

    template<typename T>
    constexpr TVector3<T>::TVector3(TVector4<T> vec4) : x(vec4.x), y(vec4.y), z(vec4.z)
    {
        xyz[3] = 0;
    }

    template<typename T>
    constexpr TVector3<T>::TVector3(TVector2<T> vec2) : x(vec2.x), y(vec2.y), z(0)
    {
        xyz[3] = 0;
    }

    template<typename T>
    TVector4<T>::TVector4(TVector2<T> vec2) : x(vec2.x), y(vec2.y), z(0), w(0)
    {}

	template<typename T>
	TVector4<T>::TVector4(TVector2<T> xy, TVector2<T> zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y)
	{}

    template<typename T>
    TVector4<T>::TVector4(TVector3<T> vec3) : x(vec3.x), y(vec3.y), z(vec3.z), w(0)
    {}

    template<typename T>
    inline TVector2<T> operator*(s32 lhs, TVector2<T> rhs)
    {
        return rhs * lhs;
    }

    template<typename T>
    inline TVector3<T> operator*(s32 lhs, TVector3<T> rhs)
    {
        return rhs * lhs;
    }
    
    template<typename T>
    inline TVector4<T> operator*(s32 lhs, TVector4<T> rhs)
    {
        return rhs * lhs;
    }

    template<typename T>
    inline TVector2<T> operator*(f32 lhs, TVector2<T> rhs)
    {
        return rhs * lhs;
    }

    template<typename T>
    inline TVector3<T> operator*(f32 lhs, TVector3<T> rhs)
    {
        return rhs * lhs;
    }

    template<typename T>
    inline TVector4<T> operator*(f32 lhs, TVector4<T> rhs)
    {
        return rhs * lhs;
    }

} // namespace CE

/// fmt user-defined Formatter for CE::Vec2
template <> struct fmt::formatter<CE::Vec2> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Vec2& vec2, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", vec2.ToString());
    }
};

/// fmt user-defined Formatter for CE::Vec3
template <> struct fmt::formatter<CE::Vec3> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Vec3& vec3, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", vec3.ToString());
    }
};

/// fmt user-defined Formatter for CE::Vec4
template <> struct fmt::formatter<CE::Vec4> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Vec4& vec4, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", vec4.ToString());
    }
};

/// fmt user-defined Formatter for CE::Vec2i
template <> struct fmt::formatter<CE::Vec2i> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Vec2i& vec2, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", vec2.ToString());
    }
};

/// fmt user-defined Formatter for CE::Vec3i
template <> struct fmt::formatter<CE::Vec3i> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Vec3i& vec3, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", vec3.ToString());
    }
};

/// fmt user-defined Formatter for CE::Vec4i
template <> struct fmt::formatter<CE::Vec4i> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Vec4i& vec4, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", vec4.ToString());
    }
};

template <> struct fmt::formatter<CE::Rect> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Rect& rect, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", rect.ToString());
    }
};

