#pragma once

namespace CE
{
    ENUM()
    enum class FShapeType : u32
    {
	    None = 0,
        Rect,
        RoundedRect,
        Circle
    };
    ENUM_CLASS(FShapeType);

    STRUCT()
    struct FUSIONCORE_API FShape final
    {
        CE_STRUCT(FShape)
    public:

        FShape() = default;

        FShape(FShapeType shapeType);

        static FShape Rect();
        static FShape Circle();

        static FShape RoundedRect(const Vec4& cornerRadius);

        FShapeType GetShapeType() const { return shapeType; }

        const Vec4& GetCornerRadius() const { return cornerRadius; }
        void SetCornerRadius(const Vec4& value) { cornerRadius = value; }

        bool operator==(const FShape& rhs) const;

        bool operator!=(const FShape& rhs) const
        {
            return !operator==(rhs);
        }

    private:

        FIELD()
        FShapeType shapeType = FShapeType::None;

        FIELD()
        Vec4 cornerRadius{};

    };

    inline FShape FRectangle() { return FShape::Rect(); }
    inline FShape FCircle() { return FShape::Circle(); }
    inline FShape FRoundedRectangle(f32 topLeft, f32 topRight, f32 bottomRight, f32 bottomLeft)
    {
	    return FShape::RoundedRect(Vec4(topLeft, topRight, bottomRight, bottomLeft));
    }
    inline FShape FRoundedRectangle(f32 cornerRadius) { return FShape::RoundedRect(Vec4(1, 1, 1, 1) * cornerRadius); }
    inline FShape FRoundedRectangle(const Vec4& cornerRadius) { return FShape::RoundedRect(cornerRadius); }
    
} // namespace CE

#include "FShape.rtti.h"