#include "FusionCore.h"

namespace CE
{

    FDockspaceSplitView::FDockspaceSplitView()
    {

    }

    void FDockspaceSplitView::Construct()
    {
        Super::Construct();

        Child(
            FNew(FOverlayStack)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
                FAssignNew(FSplitBox, splitBox),

                FNew(FStyledWidget)
                .OnPaintContentOverlay(FUNCTION_BINDING(this, OnPaintDockingPreview))
                .IgnoreHitTest(true)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill),

                FAssignNew(FDockingGuide, dockingGuide)
                .HAlign(HAlign::Center)
                .VAlign(VAlign::Center)
                .Enabled(false)
            )
        );

        dockingGuide->ownerDockspaceSplitView = this;
    }

    void FDockspaceSplitView::OnPaint(FPainter* painter)
    {
        Super::OnPaint(painter);
    }

    void FDockspaceSplitView::OnPaintDockingPreview(FPainter* painter)
    {
    	if (dockingPreviewEnabled)
        {
            FBrush background = FBrush(Color::RGBHex(0x93B8DF).WithAlpha(0.5f));
            Color borderColor = Color::RGBHex(0x5397DB);

            painter->SetBrush(background);
            painter->SetPen(FPen(borderColor, 1));

            Vec2 pos;
            Vec2 size;

            switch (dockingPreviewPosition)
            {
            case FDockingHintPosition::Center:
                pos = GetComputedPosition();
                size = GetComputedSize();
                break;
            case FDockingHintPosition::Left:
                pos = GetComputedPosition();
                size = GetComputedSize() * Vec2(0.5f, 1);
                break;
            case FDockingHintPosition::Top:
                pos = GetComputedPosition();
                size = GetComputedSize() * Vec2(1, 0.5f);
                break;
            case FDockingHintPosition::Right:
                pos = GetComputedPosition() + GetComputedSize() * Vec2(0.5f, 0);
                size = GetComputedSize() * Vec2(0.5f, 1);
                break;
            case FDockingHintPosition::Bottom:
                pos = GetComputedPosition() + GetComputedSize() * Vec2(0, 0.5f);
                size = GetComputedSize() * Vec2(1, 0.5f);
                break;
            }

            pos -= GetComputedPosition();

            painter->DrawRect(Rect::FromSize(pos, size));
        }
    }

    void FDockspaceSplitView::SetDockingPreviewEnabled(bool enabled, FDockingHintPosition position)
    {
        if (splitBox->GetChildCount() > 1)
            enabled = false;

        dockingPreviewEnabled = enabled;
        dockingPreviewPosition = position;
    }

    Ref<FDockspace> FDockspaceSplitView::GetDockspace()
    {
        if (Ref<FDockspace> dockspace = ownerDockspace.Lock())
        {
            return dockspace;
        }

        Ref<FWidget> widget = GetParent();

        while (widget != nullptr)
        {
            if (widget->IsOfType<FDockspace>())
            {
                ownerDockspace = CastTo<FDockspace>(widget);
                return ownerDockspace.Lock();
            }

            widget = widget->GetParent();
        }

        return nullptr;
    }

    void FDockspaceSplitView::SetSingleDockWindow(Ref<FDockWindow> dockWindow)
    {
        splitBox->RemoveAllChildren();

        splitBox->AddChild(dockWindow.Get());
    }

    void FDockspaceSplitView::RemoveAllContent()
    {
        splitBox->RemoveAllChildren();
    }

    void FDockspaceSplitView::SetGuideVisible(bool visible)
    {
        dockingGuide->Enabled(visible);
    }
}

