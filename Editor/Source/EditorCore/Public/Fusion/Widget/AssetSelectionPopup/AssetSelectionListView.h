#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API AssetSelectionListView : public FListView
    {
        CE_CLASS(AssetSelectionListView, FListView)
    protected:

        AssetSelectionListView();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        FListViewRow& GenerateRow();

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "AssetSelectionListView.rtti.h"
