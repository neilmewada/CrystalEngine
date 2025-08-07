#include "FusionCore.h"

namespace CE
{

    FLabel::FLabel()
    {
        m_Foreground = Colors::White;
    }

    void FLabel::CalculateIntrinsicSize()
    {
        Ref<FFusionContext> context = GetContext();
        if (!context)
        {
            Super::CalculateIntrinsicSize();
	        return;
        }

        FPainter* painter = context->GetPainter();
        if (!painter)
        {
            Super::CalculateIntrinsicSize();
	        return;
        }

        if (m_Text.IsEmpty())
        {
            intrinsicSize = painter->CalculateTextSize(" ", m_Font);
        }
        else
        {
            intrinsicSize = painter->CalculateTextSize(m_Text, m_Font);
        }

        intrinsicSize.width += m_Padding.left + m_Padding.right;
        intrinsicSize.height += m_Padding.top + m_Padding.bottom;

        ApplyIntrinsicSizeConstraints();
    }

    void FLabel::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        painter->SetFont(m_Font);
        painter->SetPen(FPen(m_Foreground));
        painter->SetBrush(FBrush());

        Vec2 size = computedSize;
        if (m_WordWrap == FWordWrap::NoWrap)
        {
            size.x = 0;
        }

        painter->DrawText(m_Text, Vec2(), size, m_WordWrap);

        if (m_Underline.GetStyle() != FPenStyle::None && m_Underline.GetColor().a > 0.001f && 
            !m_Text.IsEmpty())
        {
            FFontMetrics metrics = painter->GetFontMetrics(m_Font);

            const f32 dpi = PlatformApplication::Get()->GetSystemDpi();
            const f32 fontDpiScaling = dpi / 72.0f;
            const f32 systemDpiScaling = PlatformApplication::Get()->GetSystemDpiScaling();
            const f32 metricsScaling = fontDpiScaling / systemDpiScaling;

        	painter->GetRenderer()->CalculateUnderlinePositions(underlineRects, m_Text, m_Font, computedSize.width, m_WordWrap);

            for (int i = 0; i < underlineRects.GetSize(); ++i)
            {
                underlineRects[i] = underlineRects[i].Translate(Vec2(0,  -metrics.descender * m_Font.GetFontSize() * metricsScaling));

                painter->SetPen(m_Underline);
                painter->DrawLine(underlineRects[i].min, underlineRects[i].max);
            }
        }
    }

    FLabel::Self& FLabel::FontFamily(const CE::Name& fontFamily)
    {
        FFont copy = m_Font;
        copy.SetFamily(fontFamily);
        return Font(copy);
    }

    FLabel::Self& FLabel::FontSize(f32 fontSize)
    {
        FFont copy = m_Font;
        copy.SetFontSize(fontSize);
        return Font(copy);
    }

    FLabel::Self& FLabel::Bold(bool bold)
    {
        FFont copy = m_Font;
        copy.SetBold(bold);
        return Font(copy);
    }

    FLabel::Self& FLabel::Italic(bool italic)
    {
        FFont copy = m_Font;
        copy.SetItalic(italic);
        return Font(copy);
    }

    CE::Name FLabel::FontFamily()
    {
        return m_Font.GetFamily();
    }

    f32 FLabel::FontSize()
    {
        return m_Font.GetFontSize();
    }

    bool FLabel::Bold()
    {
        return m_Font.IsBold();
    }

    bool FLabel::Italic()
    {
        return m_Font.IsItalic();
    }

}

