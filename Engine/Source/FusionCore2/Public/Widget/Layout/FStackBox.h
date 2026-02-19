#pragma once

namespace CE
{
    ENUM()
    enum class FStackDirection
    {
		Horizontal,
		Vertical
    };
    ENUM_CLASS(FStackDirection);

    CLASS()
    class FUSIONCORE_API FStackBox : public FContainerWidget
    {
        CE_CLASS(FStackBox, FContainerWidget)
    protected:

        FStackBox();

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 

        FUSION_LAYOUT_PROPERTY(FStackDirection, StackDirection);

        FUSION_WIDGET;
    };
    
}

#include "FStackBox.rtti.h"
