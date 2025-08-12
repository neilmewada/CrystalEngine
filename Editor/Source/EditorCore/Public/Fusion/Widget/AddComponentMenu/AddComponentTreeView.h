#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API AddComponentTreeView : public FTreeView
    {
        CE_CLASS(AddComponentTreeView, FTreeView)
    protected:

        AddComponentTreeView();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        FTreeViewRow& GenerateRow();

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "AddComponentTreeView.rtti.h"
