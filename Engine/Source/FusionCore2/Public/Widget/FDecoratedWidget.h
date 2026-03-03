#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDecoratedWidget : public FCompoundWidget
    {
        CE_CLASS(FDecoratedWidget, FCompoundWidget)
    protected:

        FDecoratedWidget();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 

        FUSION_PAINT_PROPERTY(FBrush, Background);
        FUSION_PAINT_PROPERTY(FPen, Border);
        FUSION_PAINT_PROPERTY(FShape, Shape);

        FUSION_WIDGET;
    };
    
}

#include "FDecoratedWidget.rtti.h"
