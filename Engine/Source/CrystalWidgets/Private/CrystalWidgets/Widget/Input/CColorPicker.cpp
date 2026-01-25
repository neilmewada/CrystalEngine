#include "CrystalWidgets.h"

namespace CE::Widgets
{

	CColorPicker::CColorPicker()
	{
		clipChildren = false;

		receiveMouseEvents = true;
		receiveDragEvents = false;
	}

	Vec2 CColorPicker::CalculateIntrinsicSize(f32 width, f32 height)
	{
		Vec2 size = Vec2();

		static const Name imagePath = "/Engine/Assets/Textures/Data/HSVColorMap";

		RPI::Texture* colorMap = CApplication::Get()->LoadImage(imagePath);

		if (colorMap)
		{
			//return Vec2(colorMap->GetWidth(), colorMap->GetHeight());
		}

		return size;
	}

	void CColorPicker::SetNormalizedPosition(Vec2 newPosition)
	{
		this->normalizedPosition = newPosition;

		SetNeedsPaint();
	}

	void CColorPicker::Construct()
	{
		Super::Construct();


	}

	void CColorPicker::OnPaintEarly(CPaintEvent* paintEvent)
	{
		Super::OnPaintEarly(paintEvent);

		CPainter* painter = paintEvent->painter;

		Vec4 padding = computedStyle.GetPadding();

		Vec2 pos = GetComputedLayoutTopLeft() + padding.min;
		Vec2 size = GetComputedLayoutSize() - padding.min - padding.max;

		static const Name imagePath = "/Engine/Assets/Textures/Data/HSVColorMap";

		RPI::Texture* colorMap = CApplication::Get()->LoadImage(imagePath);

		if (colorMap)
		{
			CBrush brush = CBrush(Colors::White);
			painter->SetBrush(brush);

			painter->DrawTexture(Rect::FromSize(pos, size), colorMap);
		}
	}

	void CColorPicker::OnPaint(CPaintEvent* paintEvent)
	{
		Super::OnPaint(paintEvent);

		CPainter* painter = paintEvent->painter;

		Color forground = computedStyle.GetForegroundColor();
		Vec4 padding = computedStyle.GetPadding();

		CPen pen = CPen(forground);
		pen.SetWidth(2.0f);
		pen.SetStyle(CPenStyle::SolidLine);

		CBrush brush = CBrush();

		painter->SetPen(pen);
		painter->SetBrush(brush);

		Vec2 pos = GetComputedLayoutTopLeft() + padding.min;
		Vec2 size = GetComputedLayoutSize() - padding.min - padding.max;

		Vec2 drawCenter = pos + normalizedPosition * size;

		constexpr f32 circleRadius = 5;

		Rect drawRect = Rect::FromSize(drawCenter - Vec2(1, 1) * circleRadius, Vec2(1, 1) * circleRadius * 2.0f);
		painter->DrawCircle(drawRect);

		pen.SetColor(Color::RGBA(128, 128, 128));
		pen.SetWidth(1);
		painter->SetPen(pen);

		drawRect = Rect::FromSize(drawRect.min + Vec2(1, 1), drawRect.GetSize() - Vec2(1, 1) * 2);
		painter->DrawCircle(drawRect);
	}

	void CColorPicker::HandleEvent(CEvent* event)
	{
		if (event->IsMouseEvent())
		{
			CMouseEvent* mouseEvent = static_cast<CMouseEvent*>(event);

			if ((mouseEvent->type == CEventType::MousePress || mouseEvent->type == CEventType::MouseMove) && mouseEvent->button == MouseButton::Left)
			{
				Rect screenSpaceRect = GetScreenSpaceRect();

				Vec2 size = screenSpaceRect.GetSize();

				Vec2 diff = mouseEvent->mousePos - screenSpaceRect.min;

				normalizedPosition = Vec2(diff.x / size.x, diff.y / size.y);
				normalizedPosition.x = Math::Clamp01(normalizedPosition.x);
				normalizedPosition.y = Math::Clamp01(normalizedPosition.y);

				onPositionChanged(normalizedPosition);

				f32 h = normalizedPosition.x * 360;
				f32 s = Math::Clamp01(normalizedPosition.y / 0.5f);
				f32 v = 1.0f - Math::Clamp01((normalizedPosition.y - 0.5f) / 0.5f);

				onHSVColorChanged(h, s, v);

				onColorChanged(Color::HSV(h, s, v));

				mouseEvent->Consume(this);
				SetNeedsPaint();
			}
		}

		Super::HandleEvent(event);
	}

} // namespace CE::Widgets
