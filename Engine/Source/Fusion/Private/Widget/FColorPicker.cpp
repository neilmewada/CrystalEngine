#include "Fusion.h"

namespace CE
{

    FColorPicker::FColorPicker()
    {
        m_NormalizedColorPosition = Vec2(1, 1);
    }

    void FColorPicker::Construct()
    {
        Super::Construct();


    }

    void FColorPicker::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);

        static constexpr f32 CircleRadius = 3;

        {
            FBrush brush = FBrush("/Engine/Resources/Images/HSVColorMap");
            brush.SetImageFit(FImageFit::Fill);
            brush.SetBrushTiling(FBrushTiling::None);

            painter->SetBrush(brush);
            painter->SetPen(FPen());

            painter->DrawRect(Rect::FromSize(Vec2(), GetComputedSize()));
        }

        painter->SetBrush(FBrush());

        {
            FPen solid = FPen(Colors::White, 1.5f);
            painter->SetPen(solid);

            Vec2 pos = m_NormalizedColorPosition * GetComputedSize() - Vec2(1, 1) * CircleRadius;
            Vec2 size = Vec2(1, 1) * CircleRadius * 2;

            Rect drawRect = Rect::FromSize(pos, size);
            painter->DrawCircle(drawRect);

            solid = FPen(Color::RGBA(128, 128, 128), 1);
            painter->SetPen(solid);

            drawRect = Rect::FromSize(pos + Vec2(1, 1), size - Vec2(1, 1) * 2);
            painter->DrawCircle(drawRect);
        }
    }

    void FColorPicker::OnPositionEdited()
    {
        Vec2 normalizedPosition = NormalizedColorPosition();

        f32 h = normalizedPosition.x * 360;
        f32 s = Math::Clamp01(normalizedPosition.y / 0.5f);
        f32 v = 1.0f - Math::Clamp01((normalizedPosition.y - 0.5f) / 0.5f);

        m_OnHSVColorEdited(h, s, v);
        m_OnColorEdited(Color::HSV(h, s, v));
    }

    void FColorPicker::HandleEvent(FEvent* event)
    {
        if (event->IsDragEvent())
        {
            FDragEvent* dragEvent = (FDragEvent*)event;

            if (event->type == FEventType::DragBegin && dragEvent->buttons == MouseButtonMask::Left)
            {
                isDragged = true;

                Vec2 localMousePos = dragEvent->mousePosition;
                localMousePos = globalTransform.GetInverse() * Vec4(localMousePos.x, localMousePos.y, 0, 1);
                Vec2 normalizedPos = Vec2(Math::Clamp01(localMousePos.x / computedSize.x),
                    Math::Clamp01(localMousePos.y / computedSize.y));

                NormalizedColorPosition(normalizedPos);

                OnPositionEdited();

                dragEvent->draggedWidget = this;
                dragEvent->Consume(this);
            }
            else if (event->type == FEventType::DragMove)
            {
                if (isDragged)
                {
                    Vec2 localMousePos = dragEvent->mousePosition;
                    localMousePos = globalTransform.GetInverse() * Vec4(localMousePos.x, localMousePos.y, 0, 1);
                    Vec2 normalizedPos = Vec2(Math::Clamp01(localMousePos.x / computedSize.x), 
                        Math::Clamp01(localMousePos.y / computedSize.y));

                    NormalizedColorPosition(normalizedPos);

                    OnPositionEdited();

                    dragEvent->draggedWidget = this;
                    dragEvent->Consume(this);
                }
            }
            else if (event->type == FEventType::DragEnd)
            {
                isDragged = false;

                dragEvent->draggedWidget = this;
                dragEvent->Consume(this);
            }
        }

        Super::HandleEvent(event);
    }


}

