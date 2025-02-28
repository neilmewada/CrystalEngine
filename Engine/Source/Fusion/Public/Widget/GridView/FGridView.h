#pragma once

namespace CE
{
    class FGridViewItem;
    class FGridViewContainer;

    DECLARE_SCRIPT_DELEGATE(FGenerateItemDelegate, FGridViewItem&);
    DECLARE_SCRIPT_DELEGATE(FUpdateItemDelegate, void, FGridViewItem&);

    CLASS()
    class FUSION_API FGridView : public FStyledWidget
    {
        CE_CLASS(FGridView, FStyledWidget)
    protected:

        FGridView();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        Ref<FScrollBox> scrollBox;
        Ref<FGridViewContainer> container;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(FGenerateItemDelegate, OnGenerateItem);
        FUSION_PROPERTY(FUpdateItemDelegate, OnUpdateItem);

        FUSION_WIDGET;
    };
    
}

#include "FGridView.rtti.h"
