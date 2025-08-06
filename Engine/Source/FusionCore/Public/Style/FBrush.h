#pragma once
#include "Layout/LayoutTypes.h"

namespace CE
{
    ENUM()
    enum class FGradientType : u8
    {
	    Linear
    };
    ENUM_CLASS(FGradientType);

    STRUCT()
    struct FUSIONCORE_API FGradientKey final
    {
        CE_STRUCT(FGradientKey)
    public:

        FGradientKey() {}

        FGradientKey(f32 position, const Color& color) : position(position), color(color)
		{}

        FIELD()
        f32 position = 0;

        FIELD()
        Color color;

        SIZE_T GetHash() const;

    };

    STRUCT()
    struct FUSIONCORE_API FGradient final
    {
        CE_STRUCT(FGradient)
    public:

        FIELD()
        FGradientType gradientType = FGradientType::Linear;

        FIELD()
        Array<FGradientKey> stops;

        FIELD()
        float angle = 0;

        Self& AddKey(f32 position, const Color& color)
        {
            stops.Add(FGradientKey(position, color));
            return *this;
        }

        SIZE_T GetHash() const;

        bool IsValid() const
        {
            return stops.GetSize() >= 2;
        }

        bool operator==(const FGradient& rhs) const
        {
            return GetHash() == rhs.GetHash();
        }

        bool operator!=(const FGradient& rhs) const
        {
            return !operator==(rhs);
        }

    };

    ENUM()
    enum class FBrushStyle : u8
    {
	    None = 0,
        SolidFill,
        Image,
        Gradient
    };
    ENUM_CLASS(FBrushStyle);

    ENUM()
    enum class FBrushTiling : u8
    {
	    None = 0,
        TileX,
        TileY,
        TileXY
    };
    ENUM_CLASS(FBrushTiling);

    ENUM()
    enum class FImageFit : u8
    {
        Default = 0,
	    Fill,
        Contain,
        Cover
    };

    STRUCT()
    struct FUSIONCORE_API FBrush final
    {
        CE_STRUCT(FBrush)
    public:

        FBrush();

        FBrush(const Color& fillColor, FBrushStyle brushStyle = FBrushStyle::SolidFill);

        FBrush(const Name& imageName, const Color& tintColor = Colors::White);

        FBrush(const FGradient& gradient, const Color& tintColor = Colors::White);

        ~FBrush();

        bool IsValidBrush();

        FBrushStyle GetBrushStyle() const { return brushStyle; }
        FBrushTiling GetBrushTiling() const { return tiling; }
        const FGradient& GetGradient() const { return gradient; }

        void SetBrushStyle(FBrushStyle brushStyle) { this->brushStyle = brushStyle; }
        void SetBrushTiling(FBrushTiling tiling) { this->tiling = tiling; }

        const Color& GetFillColor() const { return fillColor; }
        void SetFillColor(const Color& color) { this->fillColor = color; }

        const Name& GetImageName() const { return imageName; }

        FImageFit GetImageFit() const { return imageFit; }
        void SetImageFit(FImageFit imageFit) { this->imageFit = imageFit; }

        const Vec2& GetBrushSize() const { return brushSize; }
        void SetBrushSize(Vec2 brushSize) { this->brushSize = brushSize; }

        const Vec2& GetBrushPosition() const { return brushPos; }
        void SetBrushPosition(Vec2 brushPos) { this->brushPos = brushPos; }

        bool operator==(const FBrush& rhs) const;

        bool operator!=(const FBrush& rhs) const
        {
            return !operator==(rhs);
        }

        FBrush WithTint(const Color& tintColor) const;

    private:

        FIELD()
        FGradient gradient;

    	FIELD()
    	Color fillColor;

    	FIELD()
    	Name imageName;

        FIELD()
    	Vec2 brushSize = Vec2(-1, -1); // -1 means auto size

        FIELD()
    	Vec2 brushPos = Vec2(0.5f, 0.5f); // 50% means centered

        FIELD()
    	FBrushTiling tiling = FBrushTiling::None;

        FIELD()
        FBrushStyle brushStyle = FBrushStyle::None;

        FIELD()
        FImageFit imageFit = FImageFit::Default;

        friend class FPainter;
        friend class FusionRenderer;
    };
    
} // namespace CE

#include "FBrush.rtti.h"