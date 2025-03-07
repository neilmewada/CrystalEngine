#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API AssetBrowserTreeView : public FTreeView
    {
        CE_CLASS(AssetBrowserTreeView, FTreeView)
    protected:

        AssetBrowserTreeView();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        void OnPostComputeLayout() override;

        FTreeViewRow& GenerateRow();

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "AssetBrowserTreeView.rtti.h"
