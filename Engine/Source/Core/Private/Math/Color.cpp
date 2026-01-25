#include "CoreMinimal.h"

namespace CE
{
    static const Color hueValues[] = { Color(1, 0, 0), Color(1, 1, 0), Color(0, 1, 0),
    									Color(0, 1, 1), Color(0, 0, 1), Color(1, 0, 1),
    									Color(1, 0, 0)
    };

    Color Color::HSV(f32 h, f32 s, f32 v)
    {
        Color rgb{};
        rgb.a = 1.0f;

        if (h >= 360)
            h = 359.999f;

        f32 rgbRange = v * s;
        f32 maxRGB = v;
        f32 minRGB = v - rgbRange;
        f32 hPrime = h / 60.0;
        f32 x1 = std::fmod(hPrime, 1.0);
        f32 x2 = 1.0 - std::fmod(hPrime, 1.0);

        if ((hPrime >= 0) && (hPrime < 1))
        {
            rgb.r = maxRGB;
            rgb.g = (x1 * rgbRange) + minRGB;
            rgb.b = minRGB;
        }
        else if ((hPrime >= 1) && (hPrime < 2))
        {
            rgb.r = (x2 * rgbRange) + minRGB;
            rgb.g = maxRGB;
            rgb.b = minRGB;
        }
        else if ((hPrime >= 2) && (hPrime < 3))
        {
            rgb.r = minRGB;
            rgb.g = maxRGB;
            rgb.b = (x1 * rgbRange) + minRGB;
        }
        else if ((hPrime >= 3) && (hPrime < 4))
        {
            rgb.r = minRGB;
            rgb.g = (x2 * rgbRange) + minRGB;
            rgb.b = maxRGB;
        }
        else if ((hPrime >= 4) && (hPrime < 5))
        {
            rgb.r = (x1 * rgbRange) + minRGB;
            rgb.g = minRGB;
            rgb.b = maxRGB;
        }
        else if ((hPrime >= 5) && (hPrime < 6))
        {
            rgb.r = maxRGB;
            rgb.g = minRGB;
            rgb.b = (x2 * rgbRange) + minRGB;
        }

        return rgb;
    }


} // namespace CE

