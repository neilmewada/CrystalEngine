#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <limits>

#define TO_RADIANS(Degrees) Degrees * (float)M_PI / 180.0f
#define TO_DEGREES(Radians) Radians * 180.0f / (float)M_PI

#define DEG_TO_RAD ((float)M_PI / 180.0f)
#define RAD_TO_DEG (180.0f / (float)M_PI)

namespace CE
{

    class CORE_API Math
    {
    public:
        Math() = delete;
        ~Math() = delete;

        static constexpr f32 PI = M_PI;

        static u16 ToFloat16(f32 floatValue)
        {
            f32* ptr = &floatValue;
            unsigned int fltInt32 = *((u32*)ptr);

            u16 fltInt16 = (fltInt32 >> 31) << 5;
            u16 tmp = (fltInt32 >> 23) & 0xff;
            tmp = (tmp - 0x70) & ((unsigned int)((int)(0x70 - tmp) >> 4) >> 27);
            fltInt16 = (fltInt16 | tmp) << 10;
            fltInt16 |= (fltInt32 >> 13) & 0x3ff;

            return fltInt16;
        }

        // Needs to be tested!
        static f32 ToFloat32(u16 float16)
        {
            uint32_t t1;
            uint32_t t2;
            uint32_t t3;

            t1 = float16 & 0x7fffu;                       // Non-sign bits
            t2 = float16 & 0x8000u;                       // Sign bit
            t3 = float16 & 0x7c00u;                       // Exponent

            t1 <<= 13u;                              // Align mantissa on MSB
            t2 <<= 16u;                              // Shift sign bit into position

            t1 += 0x38000000;                       // Adjust bias

            t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

            t1 |= t2;                               // Re-insert sign bit

            return *(f32*)(&t1);
        }

        static bool ApproxEquals(f32 a, f32 b) { return fpclassify(a - b) == FP_ZERO; }
        static bool ApproxEquals(f64 a, f64 b) { return fpclassify(a - b) == FP_ZERO; }
        static bool ApproxEquals(s32 a, s32 b) { return a == b; }

        static constexpr f32 Infinity() { return NumericLimits<f32>::Infinity(); }

        CE_INLINE static f32 ToDegrees(f32 radians) { return TO_DEGREES(radians); }
        CE_INLINE static f32 ToRadians(f32 degrees) { return TO_RADIANS(degrees); }

        CE_INLINE static f32 Sin(f32 radians) { return sin(radians); }
        CE_INLINE static f32 ASin(f32 sine) { return asin(sine); }

        CE_INLINE static f32 Cos(f32 radians) { return cos(radians); }
        CE_INLINE static f32 ACos(f32 cosine) { return acos(cosine); }

        CE_INLINE static f32 Tan(f32 radians) { return tan(radians); }
        CE_INLINE static f32 ATan(f32 tangent) { return atan(tangent); }

        CE_INLINE static f32 Abs(f32 value) { return abs(value); }
        CE_INLINE static f64 Abs(f64 value) { return abs(value); }

        CE_INLINE static s32 Abs(s32 value) { return abs(value); }
        CE_INLINE static s64 Abs(s64 value) { return abs(value); }

        CE_INLINE static f32 Sqrt(f32 value) { return sqrt(value); }

        template<typename T1, typename T2>
        INLINE static auto Pow(T1 base, T2 power) { return pow(base, power); }

        template<typename T>
        constexpr static T Min(std::initializer_list<T> list)
        {
            auto min = std::numeric_limits<T>::max();

            for (auto entry : list)
            {
                if (entry < min)
                    min = entry;
            }

            return min;
        }

        template<typename T>
        constexpr static T Max(std::initializer_list<T> list)
        {
            auto max = std::numeric_limits<T>::min();
            
            for (auto entry : list)
            {
                if (entry > max)
                    max = entry;
            }

            return max;
        }

        static f32 Round(f32 value)
        {
            return std::round(value);
        }

        static f64 Round(f64 value)
        {
            return std::round(value);
        }

        static s32 RoundToInt(f32 value)
        {
            return std::lround(value);
        }

        static s64 RoundToInt64(f32 value)
        {
            return std::llround(value);
        }

        static s32 RoundToInt(f64 value)
        {
            return std::lround(value);
        }

        static s64 RoundToInt64(f64 value)
        {
            return std::llround(value);
        }

        template<typename T>
        CE_INLINE static T Min(T a, T b)
        {
            return a < b ? a : b;
        }

        template<typename T>
        CE_INLINE static T Max(T a, T b)
        {
            return a > b ? a : b;
        }

        template<typename T>
        CE_INLINE static T Clamp(T value, T min, T max)
        {
            if (min > max)
                std::swap(min, max);
	        return Min(Max(value, min), max);
        }

        template<typename T>
        CE_INLINE static T Clamp01(T value) { return Clamp<T>(value, 0, 1); }

        CE_INLINE static f32 Lerp(f32 from, f32 to, f32 t) { return from * (1 - Clamp01(t)) + to * Clamp01(t); }

        CE_INLINE static f32 LerpUnclamped(f32 from, f32 to, f32 t) { return from * (1 - t) + to * (t); }
        
    private:

    };

	template<typename T>
	FORCE_INLINE bool IsNan(T value)
	{
		return std::isnan(value);
	}
    
} // namespace CE

