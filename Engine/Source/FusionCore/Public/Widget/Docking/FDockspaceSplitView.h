#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockspaceSplitView : public FStyledWidget
    {
        CE_CLASS(FDockspaceSplitView, FStyledWidget)
    protected:

        FDockspaceSplitView();

        void Construct() override;

        void OnPaint(FPainter* painter) override;

        FUNCTION()
        void OnPaintDockingPreview(FPainter* painter);

    public: // - Public API -

        bool SupportsDropTarget() const override { return true; }

        void SetDockingPreviewEnabled(bool enabled, FDockingHintPosition position);

        bool IsDockingPreviewEnabled() { return dockingPreviewEnabled; }

        FDockingHintPosition GetDockingPreviewHintPosition() { return dockingPreviewPosition; }

        Ref<FDockspace> GetDockspace();

        void SetSingleDockWindow(Ref<FDockWindow> dockWindow);

        void RemoveAllContent();

        void SetGuideVisible(bool visible);

    protected: // - Internal -

        Array<Ref<FDockspaceSplitView>> childrenSplitViews;

        Ref<FSplitBox> splitBox;
        Ref<FDockingGuide> dockingGuide;

        Ref<FDockWindow> contentWindow;

        WeakRef<FDockspace> ownerDockspace;

        bool dockingPreviewEnabled = false;
        FDockingHintPosition dockingPreviewPosition = FDockingHintPosition::Center;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FDockspace;
    };
    
}

#include "FDockspaceSplitView.rtti.h"
