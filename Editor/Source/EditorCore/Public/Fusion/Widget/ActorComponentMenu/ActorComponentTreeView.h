#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API ActorComponentTreeView : public FTreeView
    {
        CE_CLASS(ActorComponentTreeView, FTreeView)
    protected:

        ActorComponentTreeView();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        FTreeViewRow& GenerateRow();

        

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "ActorComponentTreeView.rtti.h"
