#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FListWidgetContainer : public FContainerWidget
    {
        CE_CLASS(FListWidgetContainer, FContainerWidget)
    public:

        FListWidgetContainer();

        virtual ~FListWidgetContainer();

        // - Public API -

        void CalculateIntrinsicSize() override;

        void PlaceSubWidgets() override;

    protected:

        void OnPaint(FPainter* painter) override;

    public: // - Fusion Properties - 

        FUSION_LAYOUT_PROPERTY(f32, Gap);

        FUSION_WIDGET;
    };
    
}

#include "FListWidgetContainer.rtti.h"
