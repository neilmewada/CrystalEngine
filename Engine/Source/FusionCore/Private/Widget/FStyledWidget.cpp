#include "FusionCore.h"

namespace CE
{

    FStyledWidget::FStyledWidget()
    {
        m_BackgroundShape = FRectangle();
        m_Opacity = 1.0f;
        m_ClipShape = FShapeType::None;
        m_BorderStyle = FPenStyle::SolidLine;
    }

    void FStyledWidget::CalculateIntrinsicSize()
    {
	    Super::CalculateIntrinsicSize();
    }

    void FStyledWidget::OnPaint(FPainter* painter)
    {
        ZoneScoped;

        if (m_Opacity < 0.001f)
            return;

        if (m_Opacity < 1.0f)
        {
	        painter->PushOpacity(m_Opacity);
        }

        if (m_ClipShape.GetShapeType() != FShapeType::None)
        {
            painter->PushClipRect(localTransform, computedSize);
        }

	    if ((m_BackgroundShape.GetShapeType() != FShapeType::None && m_Background.GetBrushStyle() != FBrushStyle::None) ||
            (m_BorderWidth > 0 && m_BorderColor.a > 0))
	    {
            painter->SetBrush(m_Background);

            if (m_BorderWidth > 0 && m_BorderColor.a > 0)
            {
                painter->SetPen(FPen(m_BorderColor, m_BorderWidth, m_BorderStyle));
            }
            else
            {
                painter->SetPen(FPen());
            }

            switch (m_Background.GetBrushStyle())
            {
            case FBrushStyle::None:
                break;
            case FBrushStyle::SolidFill:
                break;
            case FBrushStyle::Image:
                break;
            case FBrushStyle::Gradient:
                break;
            }

            isCulled = !painter->DrawShape(Rect::FromSize(Vec2(), computedSize), m_BackgroundShape);
	    }
        else
        {
            isCulled = painter->IsCulled(Vec2(), computedSize);
        }

        OnPaintContent(painter);

        // Paint child widgets
	    Super::OnPaint(painter);

        m_OnPaintContentOverlay.Broadcast(painter);

        if (m_ClipShape.GetShapeType() != FShapeType::None)
        {
            painter->PopClipRect();
        }

        if (m_Opacity < 1.0f)
        {
	        painter->PopOpacity();
        }
    }

    void FStyledWidget::SetContextRecursively(FFusionContext* context)
    {
	    Super::SetContextRecursively(context);

        ApplyStyle();
    }

    void FStyledWidget::OnAttachedToParent(FWidget* parent)
    {
	    Super::OnAttachedToParent(parent);

    }

}

