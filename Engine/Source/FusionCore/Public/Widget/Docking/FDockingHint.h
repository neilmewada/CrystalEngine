#pragma once

namespace CE
{
    class FDockingGuide;
    class FDockspaceSplitView;

    ENUM()
    enum class FDockingHintPosition
    {
	    Center,
        Left, Top, Right, Bottom
    };
    ENUM_CLASS(FDockingHintPosition);

    CLASS()
    class FUSIONCORE_API FDockingHint : public FStyledWidget
    {
        CE_CLASS(FDockingHint, FStyledWidget)
    protected:

        FDockingHint();

        void Construct() override;

        void OnAttachedToParent(FWidget* parent) override;

        void HandleEvent(FEvent* event) override;

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDropTarget() const override;

        bool IsHovered() const { return isHovered; }

        Ref<FDockingGuide> GetOwnerGuide();

    protected: // - Internal -

        Ref<FStyledWidget> innerWidget;
        Ref<FImage> icon;
        bool isHovered = false;

        WeakRef<FDockingGuide> ownerGuide;
        WeakRef<FDockspaceSplitView> hintDockspaceSplit;

        FDockingHintPosition hintPosition = FDockingHintPosition::Center;

    public: // - Fusion Properties - 

        Self& HintPosition(FDockingHintPosition position);

        FDockingHintPosition HintPosition();

        FUSION_WIDGET;
    };
    
}

#include "FDockingHint.rtti.h"
