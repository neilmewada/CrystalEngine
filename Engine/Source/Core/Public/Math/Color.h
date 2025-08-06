#pragma once

namespace CE
{
    
    class Color
    {
    public:
        constexpr Color() : a(0), b(0), g(0), r(0)
        {

        }

        constexpr Color(f32 r, f32 g, f32 b, f32 a)
            : a(a), b(b), g(g), r(r)
        {
            
        }

        constexpr Color(f32 r, f32 g, f32 b)
            : a(1), b(b), g(g), r(r)
        {

        }

        constexpr Color(Vec4 v)
            : a(v.w), b(v.z), g(v.y), r(v.x)
        {

        }

        constexpr Color(Vec3 v)
            : a(1), b(v.z), g(v.y), r(v.x)
        {

        }

		static constexpr Color RGBA8(u8 r, u8 g, u8 b, u8 a = (u8)255)
        {
            return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        }

        static constexpr Color RGBHex(u32 hex)
        {
            return RGBA8((u8)(hex >> 16), (u8)(hex >> 8), (u8)(hex));
        }

		static constexpr Color RGBAHex(u32 hex)
        {
            return RGBA8((u8)(hex >> 24), (u8)(hex >> 16), (u8)(hex >> 8), (u8)(hex));
        }

        CORE_API static Color HSV(f32 h, f32 s, f32 v);

		static constexpr Color RGBA(u8 r, u8 g, u8 b, u8 a = (u8)255)
		{
			return RGBA8(r, g, b, a);
		}

        constexpr bool operator==(const Color& rhs) const
		{
            return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
		}

        constexpr bool operator!=(const Color& rhs) const
        {
            return operator==(rhs);
        }

        constexpr Color operator*(s32 value) const
        {
            return Color(value * r, value * g, value * b, value * a);
        }

        constexpr Color operator*(u32 value) const
        {
            return Color(value * r, value * g, value * b, value * a);
        }

        constexpr Color operator*(f32 value) const
        {
            return Color(value * r, value * g, value * b, value * a);
        }

        constexpr Color operator*=(s32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr Color operator*=(u32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr Color operator*=(f32 value)
        {
            *this = *this * value;
            return *this;
        }

        constexpr Color operator/(f32 value) const
        {
            return Color(r / value, g / value, b / value, a / value);
        }

        constexpr Color operator/=(f32 value)
        {
            *this = *this / value;
            return *this;
        }

        f32 operator[](u32 index)
        {
            if (index == 0)
            {
                return r;
            }
            else if (index == 1)
            {
                return g;
            }
            else if (index == 2)
            {
                return b;
            }
            else if (index == 3)
            {
                return a;
            }
            return 0;
        }
        
        constexpr u32 ToU32() const
        {
            return ((u32)(r * 255)) | ((u32)(g * 255) << 8) | ((u32)(b * 255) << 16) | ((u32)(a * 255) << 24);
        }
        
        constexpr Vec4 ToVec4() const
        {
            return Vec4(r, g, b, a);
        }

        constexpr Vec3 ToHSV() const
        {
            f32 cmax = std::max(r, std::max(g, b)); // maximum of r, g, b 
            f32 cmin = std::min(r, std::min(g, b)); // minimum of r, g, b 
            f32 diff = cmax - cmin; // diff of cmax and cmin. 
            f32 h = -1, s = -1;

            // if cmax and cmax are equal then h = 0 
            if (cmax == cmin)
                h = 0;

            // if cmax equal r then compute h 
            else if (cmax == r)
                h = fmod(60 * ((g - b) / diff) + 360, 360);

            // if cmax equal g then compute h 
            else if (cmax == g)
                h = fmod(60 * ((b - r) / diff) + 120, 360);

            // if cmax equal b then compute h 
            else if (cmax == b)
                h = fmod(60 * ((r - g) / diff) + 240, 360);

            // if cmax equal zero 
            if (cmax == 0)
                s = 0;
            else
                s = (diff / cmax) * 100;

            f32 v = cmax * 100;

            return Vec3(h, s / 100.0f, v / 100.0f);
        }

		inline static Color Lerp(const Color& from, const Color& to, f32 t)
		{
			return Color(Math::Lerp(from.r, to.r, t), Math::Lerp(from.g, to.g, t), Math::Lerp(from.b, to.b, t), Math::Lerp(from.a, to.a, t));
		}

        inline SIZE_T GetHash() const
        {
            SIZE_T hash = CE::GetHash(r);
            CombineHash(hash, g);
            CombineHash(hash, b);
            CombineHash(hash, a);
            return hash;
        }

        inline Color WithAlpha(f32 alpha) const
        {
            return Color(r, g, b, alpha);
        }

    public:

        union {
            struct {
				f32 a, b, g, r;
            };
            f32 rgba[4] = { 0, 0, 0, 0 };
        };
    };

    namespace Colors
    {
        // Primary
        constexpr Color Red = Color(1.0f, 0.0f, 0.0f, 1.0f);
        constexpr Color Green = Color(0.0f, 1.0f, 0.0f, 1.0f);
        constexpr Color Blue = Color(0.0f, 0.0f, 1.0f, 1.0f);

        // Secondary
        constexpr Color Yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);
        constexpr Color Cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
        constexpr Color Magenta = Color(1.0f, 0.0f, 1.0f, 1.0f);

        // Greyscale
        constexpr Color White = Color(1.0f, 1.0f, 1.0f, 1.0f);
        constexpr Color Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
        constexpr Color Gray = Color(0.5f, 0.5f, 0.5f, 1.0f);
        constexpr Color LightGray = Color(0.75f, 0.75f, 0.75f, 1.0f);
        constexpr Color DarkGray = Color(0.25f, 0.25f, 0.25f, 1.0f);
        constexpr Color Clear = Color(0.0f, 0.0f, 0.0f, 0.0f);

        // Extra (Editor / Debug / Stylized)
        constexpr Color Orange = Color(1.0f, 0.6f, 0.0f, 1.0f);
        constexpr Color Purple = Color(0.5f, 0.0f, 0.5f, 1.0f);
        constexpr Color Pink = Color(1.0f, 0.4f, 0.7f, 1.0f);
        constexpr Color Lime = Color(0.5f, 1.0f, 0.0f, 1.0f);
        constexpr Color Teal = Color(0.0f, 0.5f, 0.5f, 1.0f);
        constexpr Color Olive = Color(0.5f, 0.5f, 0.0f, 1.0f);
        constexpr Color Maroon = Color(0.5f, 0.0f, 0.0f, 1.0f);
        constexpr Color Navy = Color(0.0f, 0.0f, 0.5f, 1.0f);
        constexpr Color Gold = Color(1.0f, 0.84f, 0.0f, 1.0f);
        constexpr Color SkyBlue = Color(0.53f, 0.81f, 0.92f, 1.0f);
    }

} // namespace CE

