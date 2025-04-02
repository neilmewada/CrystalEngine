#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FScrollBox : public FCompoundWidget
    {
        CE_CLASS(FScrollBox, FCompoundWidget)
    public:

        FScrollBox();

        void CalculateIntrinsicSize() override;

        void PlaceSubWidgets() override;

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDragEvents() const override { return true; }

        bool CapturesMouseWheel() const override { return true; }

    protected:

        void OnPostComputeLayout() override;

        Rect GetVerticalScrollBarRect();
        Rect GetHorizontalScrollBarRect();

        FWidget* HitTest(Vec2 mousePosition) override;

        void OnPaint(FPainter* painter) override;

        void HandleEvent(FEvent* event) override;

    public: // - Fusion Properties - 

        FUSION_LAYOUT_PROPERTY(bool, VerticalScroll);
        FUSION_LAYOUT_PROPERTY(bool, HorizontalScroll);

        FUSION_LAYOUT_PROPERTY(bool, HideVerticalScroll);
        FUSION_LAYOUT_PROPERTY(bool, HideHorizontalScroll);

        FUSION_PROPERTY(FBrush, Background);
        FUSION_PROPERTY(Color, BorderColor);
        FUSION_PROPERTY(f32, BorderWidth);

        FUSION_PROPERTY(FBrush, ScrollBarBackground);
        FUSION_PROPERTY(FPen, ScrollBarBackgroundPen);
        FUSION_PROPERTY(FBrush, ScrollBarBrush);
        FUSION_PROPERTY(FBrush, ScrollBarHoverBrush);
        FUSION_PROPERTY(FPen, ScrollBarPen);
        FUSION_PROPERTY(FPen, ScrollBarHoverPen);
        FUSION_PROPERTY(FShape, ScrollBarShape);
        FUSION_LAYOUT_PROPERTY(float, ScrollBarWidth);
        FUSION_LAYOUT_PROPERTY(float, ScrollBarMargin);

        FUSION_PROPERTY(f32, VerticalScrollSensitivity);
        FUSION_PROPERTY(f32, HorizontalScrollSensitivity);

        FUSION_EVENT(ScriptEvent<void(FScrollBox*)>, OnBackgroundClicked);

        Self& NormalizedScrollY(f32 value);
        f32 NormalizedScrollY();

        Self& NormalizedScrollX(f32 value);
        f32 NormalizedScrollX();

        void ClampTranslation();

    private:

        bool isVerticalScrollVisible = false;
        bool isHorizontalScrollVisible = false;

        bool isVerticalScrollDragged = false;
        bool isHorizontalScrollDragged = false;

        bool isVerticalScrollHighlighted = false;
        bool isHorizontalScrollHighlighted = false;

        FUSION_WIDGET;
    };
    
}

#include "FScrollBox.rtti.h"
