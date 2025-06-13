#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockspaceSplitView : public FSplitBox
    {
        CE_CLASS(FDockspaceSplitView, FSplitBox)
    protected:

        FDockspaceSplitView();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        Array<Ref<FDockspaceSplitView>> childrenSplitViews;

        Ref<FDockWindow> contentWindow;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
        friend class FDockspace;
    };
    
}

#include "FDockspaceSplitView.rtti.h"
