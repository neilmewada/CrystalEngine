#include "CrystalWidgets.h"

namespace CE::Widgets
{

    CLabel::CLabel()
    {
        receiveDragEvents = receiveMouseEvents = false;
        allowVerticalScroll = allowHorizontalScroll = false;
        clipChildren = false;
    }

    CLabel::~CLabel()
    {

    }

    Vec2 CLabel::CalculateIntrinsicSize(f32 width, f32 height)
    {
        if (text.IsEmpty())
            return Vec2();

	    Vec2 base = Super::CalculateIntrinsicSize(width, height);

        Renderer2D* renderer = nullptr;
        CWindow* owner = ownerWindow;
        while (owner != nullptr)
        {
            if (owner->ownerWindow == nullptr)
            {
                renderer = owner->GetNativeWindow()->renderer;
	            break;
            }
            owner = owner->ownerWindow;
        }

        if (!renderer)
            return base;

        Name fontName = computedStyle.properties[CStylePropertyType::FontName].string;

        f32 fontSize = 14;
        if (computedStyle.properties.KeyExists(CStylePropertyType::FontSize))
            fontSize = computedStyle.properties[CStylePropertyType::FontSize].single;

        if (fontSize < 8)
            fontSize = 8;

        return renderer->CalculateTextSize(text, fontSize, fontName, 0);
    }

    void CLabel::SetText(const String& text)
    {
        this->text = text;

        SetNeedsLayout();
        SetNeedsPaint();
    }

    void CLabel::OnPaint(CPaintEvent* paintEvent)
    {
        Super::OnPaint(paintEvent);

        if (text.IsEmpty())
            return;

        CPainter* painter = paintEvent->painter;

        Name fontName = computedStyle.GetFontName();

        f32 fontSize = 14;
        if (computedStyle.properties.KeyExists(CStylePropertyType::FontSize))
            fontSize = computedStyle.properties[CStylePropertyType::FontSize].single;

        Color color = Colors::White;

        if (computedStyle.properties.KeyExists(CStylePropertyType::Foreground))
            color = computedStyle.properties[CStylePropertyType::Foreground].color;

        CWordWrap wordWrap = CWordWrap::BreakWord;
        if (computedStyle.properties.KeyExists(CStylePropertyType::WordWrap))
            wordWrap = (CWordWrap)computedStyle.properties[CStylePropertyType::WordWrap].enumValue.x;

        CTextDecoration decoration = computedStyle.GetTextDecoration();

        CFont originalFont = painter->GetFont();
        CPen originalPen = painter->GetPen();

        CFont font = CFont(fontName, (u32)fontSize, false);
		font.SetUnderline(EnumHasFlag(decoration.linePosition, CTextDecorationLine::Underline));
        switch (decoration.lineStyle)
        {
        case CTextDecorationStyle::Solid:
            font.SetLineStyle(CPenStyle::SolidLine);
	        break;
        case CTextDecorationStyle::Dashed:
            font.SetLineStyle(CPenStyle::DashedLine);
	        break;
        case CTextDecorationStyle::Dotted:
            font.SetLineStyle(CPenStyle::DottedLine);
            break;
        }
        
        font.SetLineColor(decoration.lineColor);

        CPen pen = CPen(color);
        pen.SetWidth(decoration.thickness);

        painter->SetFont(font);
        painter->SetPen(pen);

        Rect rect = Rect::FromSize(GetComputedLayoutTopLeft(), GetComputedLayoutSize());
        
        if (wordWrap == CWordWrap::Normal) // Go to new line when a word goes out of bounds
            painter->DrawText(text, rect);
        else // Just clip the word that goes out of bounds
            painter->DrawText(text, rect.min);

        painter->SetFont(originalFont);
        painter->SetPen(originalPen);
    }

    
} // namespace CE::Widgets
