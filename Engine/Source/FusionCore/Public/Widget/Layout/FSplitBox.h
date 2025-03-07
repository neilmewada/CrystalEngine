#pragma once

namespace CE
{
    class FSplitBox;

    DECLARE_SCRIPT_EVENT(FSplitBoxEvent, FSplitBox*);

    ENUM()
    enum class FSplitDirection
    {
	    Horizontal,
        Vertical
    };
    ENUM_CLASS(FSplitDirection);

    CLASS()
    class FUSIONCORE_API FSplitBox : public FContainerWidget
    {
        CE_CLASS(FSplitBox, FContainerWidget)
    public:

        FSplitBox();

        void CalculateIntrinsicSize() override;

        void PlaceSubWidgets() override;

        bool SupportsDragEvents() const override { return true; }

        bool SupportsMouseEvents() const override { return true; }

        int GetDraggedSplitIndex() const { return draggedSplitIndex; }

    protected:

        void Construct() override;

        void OnPaint(FPainter* painter) override;

        void HandleEvent(FEvent* event) override;

    public: // - Fusion Properties -

        FUSION_PROPERTY(Color, SplitterHoverBackground);
        FUSION_PROPERTY(Color, SplitterBackground);

        FUSION_LAYOUT_PROPERTY(FSplitDirection, Direction);
        FUSION_LAYOUT_PROPERTY(CE::HAlign, ContentHAlign);
        FUSION_LAYOUT_PROPERTY(CE::VAlign, ContentVAlign);
        FUSION_LAYOUT_PROPERTY(f32, SplitterSize);
        FUSION_LAYOUT_PROPERTY(f32, SplitterDrawRatio);

        FUSION_EVENT(FSplitBoxEvent, OnSplitterDragged);

    private:

        int draggedSplitIndex = -1;
        int hoveredSplitIndex = -1;
        Vec2 dragStartMousePos;
        f32 leftFillRatio = 0;
        f32 rightFillRatio = 0;
        bool isCursorPushed = false;

        FUSION_WIDGET;
    };
    
}

#include "FSplitBox.rtti.h"
