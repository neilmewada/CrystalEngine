#include "FusionCore.h"

namespace CE
{

    FDockingHint::FDockingHint()
    {

    }

    constexpr f32 FixedSize = 40;
    constexpr f32 InnerSize = FixedSize * 0.6f;

    void FDockingHint::Construct()
    {
        Super::Construct();

        Width(FixedSize);
        Height(FixedSize);

        Background(Color::RGBA(90, 90, 90));
        Border(Color::RGBA(70, 70, 70), 1);

        Child(
			FAssignNew(FStyledWidget, innerWidget)
            .Background(Color::RGBHex(0x72AAE4))
            .Border(Color::RGBHex(0x3087DE), 1)
            .Width(InnerSize)
            .Height(InnerSize)
            .Margin(Vec4(0, 0, 0, InnerSize * 0.5f))
            .HAlign(HAlign::Center)
            .VAlign(VAlign::Center)
            (
                FAssignNew(FImage, icon)
                .Background(FBrush("/Engine/Resources/Icons/CaretDown"))
                .Width(8)
                .Height(8)
                .HAlign(HAlign::Center)
                .VAlign(VAlign::Center)
            )
        );
    }

    void FDockingHint::OnAttachedToParent(FWidget* parent)
    {
	    Super::OnAttachedToParent(parent);

        Ref<FWidget> widget = parent;

        ownerGuide = nullptr;

        while (widget != nullptr)
        {
            if (widget->IsOfType<FDockingGuide>())
            {
                ownerGuide = CastTo<FDockingGuide>(widget);
	            break;
            }

            widget = widget->GetParent();
        }
    }

    void FDockingHint::HandleEvent(FEvent* event)
    {
        if (event->IsMouseEvent() && event->sender == this)
        {
            FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

            if (event->type == FEventType::MouseEnter)
            {
                Background(Color::RGBA(0, 112, 224));
                isHovered = true;

                if (Ref<FDockingGuide> guide = GetOwnerGuide())
                {
	                if (Ref<FDockspaceSplitView> dockspaceSplit = guide->GetDockspaceSplitView())
	                {
                        if (Ref<FDockspace> dockspace = dockspaceSplit->GetDockspace())
                        {
                            if (dockspace->AllowSplitting())
                            {
                                hintDockspaceSplit = dockspaceSplit;
                                dockspaceSplit->SetDockingPreviewEnabled(true, hintPosition);
                            }
                        }
	                }
                }
            }
            else if (event->type == FEventType::MouseLeave)
            {
                Background(Color::RGBA(90, 90, 90));
                isHovered = false;

                if (Ref<FDockspaceSplitView> dockspaceSplit = hintDockspaceSplit.Lock())
                {
                    dockspaceSplit->SetDockingPreviewEnabled(false, hintPosition);
                    hintDockspaceSplit = nullptr;
                }
            }
        }

	    Super::HandleEvent(event);
    }

    bool FDockingHint::SupportsDropTarget() const
    {
        return true;
    }

    Ref<FDockingGuide> FDockingHint::GetOwnerGuide()
    {
        if (Ref<FDockingGuide> owner = ownerGuide.Lock())
        {
            return owner;
        }

        Ref<FWidget> widget = GetParent();

        while (widget != nullptr)
        {
	        if (widget->IsOfType<FDockingGuide>())
	        {
                ownerGuide = CastTo<FDockingGuide>(widget);
                return ownerGuide.Lock();
	        }

            widget = widget->GetParent();
        }

        return nullptr;
    }

    FDockingHint::Self& FDockingHint::HintPosition(FDockingHintPosition position)
    {
        icon->Enabled(true);

        hintPosition = position;

	    switch (position)
	    {
	    case FDockingHintPosition::Center:
            (*innerWidget)
                .Width(InnerSize)
                .Height(InnerSize)
                .Margin(Vec4(0, 0, 0, 0));
            icon->Enabled(false);
		    break;
	    case FDockingHintPosition::Left:
            (*innerWidget)
                .Width(InnerSize * 0.5f)
                .Height(InnerSize)
                .Margin(Vec4(0, 0, InnerSize * 0.5f, 0));
            icon->Angle(90);
		    break;
	    case FDockingHintPosition::Top:
            (*innerWidget)
                .Width(InnerSize)
                .Height(InnerSize * 0.5f)
                .Margin(Vec4(0, 0, 0, InnerSize * 0.5f));
            icon->Angle(180);
		    break;
	    case FDockingHintPosition::Right:
            (*innerWidget)
                .Width(InnerSize * 0.5f)
                .Height(InnerSize)
                .Margin(Vec4(InnerSize * 0.5f, 0, 0, 0));
            icon->Angle(-90);
		    break;
	    case FDockingHintPosition::Bottom:
            (*innerWidget)
                .Width(InnerSize)
                .Height(InnerSize * 0.5f)
                .Margin(Vec4(0, InnerSize * 0.5f, 0, 0));
            icon->Angle(0);
		    break;
	    }

        return *this;
    }
    FDockingHintPosition FDockingHint::HintPosition()
    {
        return hintPosition;
    }
}

