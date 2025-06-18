#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FReorderableStack : public FContainerWidget
    {
        CE_CLASS(FReorderableStack, FContainerWidget)
    protected:

        FReorderableStack();

        void Construct() override;

    public: // - Public API -

        void UpdateOrder();

        void CalculateIntrinsicSize() override;

        void PlaceSubWidgets() override;

        void OnPaint(FPainter* painter) override;

        FWidget* HitTest(Vec2 localMousePos) override;

        // For internal use only!
		void SetActiveItem(Ref<FReorderableStackItem> item);

    protected: // - Internal -

        void OnActiveItemDragged(FDragEvent* dragEvent);

        virtual void OnItemsRearranged() {}

        Ref<FReorderableStackItem> activeItem;

    public: // - Fusion Properties -

        FUSION_LAYOUT_PROPERTY(FStackBoxDirection, Direction);
        FUSION_LAYOUT_PROPERTY(CE::HAlign, ContentHAlign);
        FUSION_LAYOUT_PROPERTY(CE::VAlign, ContentVAlign);
        FUSION_LAYOUT_PROPERTY(f32, Gap);

        FUSION_PROPERTY(FShape, BackgroundShape);
        FUSION_PROPERTY(FBrush, Background);
        FUSION_PROPERTY(Color, BorderColor);
        FUSION_PROPERTY(f32, BorderWidth);
        FUSION_PROPERTY(FPenStyle, BorderStyle);

        Self& Border(const Color& borderColor, f32 borderWidth = 1.0f)
        {
            return (*this)
                .BorderColor(borderColor)
                .BorderWidth(borderWidth);
        }

        FUSION_WIDGET;
        friend class FReorderableStackItem;
    };
    
}

#include "FReorderableStack.rtti.h"
