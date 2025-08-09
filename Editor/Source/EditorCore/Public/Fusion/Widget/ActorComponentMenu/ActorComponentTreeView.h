#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API ActorComponentTreeView : public FListView
    {
        CE_CLASS(ActorComponentTreeView, FListView)
    protected:

        ActorComponentTreeView();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        FListViewRow& GenerateRow();

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "ActorComponentTreeView.rtti.h"
