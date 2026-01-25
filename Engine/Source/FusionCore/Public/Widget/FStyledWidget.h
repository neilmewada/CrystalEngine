#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FStyledWidget : public FCompoundWidget
    {
        CE_CLASS(FStyledWidget, FCompoundWidget)
    public:

        FStyledWidget();

        // - Public API -

        void CalculateIntrinsicSize() override;

        void SetContextRecursively(FFusionContext* context) override;

        // - Events -

        void OnPaint(FPainter* painter) override;

    protected:

        //! @brief Called before the content is being painted but after the background and border of this widget.
        virtual void OnPaintContent(FPainter* painter) {}

        void OnAttachedToParent(FWidget* parent) override;

    protected: // - Fusion Fields -

    public:   // - Fusion Properties -

        FUSION_PROPERTY(FBrush, Background);
        FUSION_PROPERTY(FShape, BackgroundShape);
        FUSION_PROPERTY(FShape, ClipShape);

        const Vec4& CornerRadius() const
        {
            return m_BackgroundShape.GetCornerRadius();
        }

        Self& CornerRadius(const Vec4& cornerRadius)
        {
            return BackgroundShape(FShape::RoundedRect(cornerRadius));
        }

        Self& CornerRadius(f32 cornerRadius)
        {
            return BackgroundShape(FRoundedRectangle(cornerRadius));
        }

        FUSION_PROPERTY(Color, BorderColor);
        FUSION_PROPERTY(f32, BorderWidth);
        FUSION_PROPERTY(FPenStyle, BorderStyle);

        FUSION_EVENT(FPaintEvent, OnPaintContentOverlay);

        Self& Border(const Color& borderColor, f32 borderWidth = 1.0f)
        {
            return (*this)
        		.BorderColor(borderColor)
				.BorderWidth(borderWidth);
        }

        FUSION_PROPERTY(f32, Opacity);


        FUSION_WIDGET;
    };
    
}

#include "FStyledWidget.rtti.h"
