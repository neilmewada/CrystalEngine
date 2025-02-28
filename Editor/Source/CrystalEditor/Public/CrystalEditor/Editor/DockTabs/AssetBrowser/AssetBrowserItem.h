#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API AssetBrowserItem : public FSelectableButton
    {
        CE_CLASS(AssetBrowserItem, FSelectableButton)
    protected:

        AssetBrowserItem();

        void Construct() override;

        void OnSelected() override;
        void OnDeselected() override;

        void HandleEvent(FEvent* event) override;

    public: // - Public API -


    protected: // - Internal -

        Ref<FLabel> titleLabel;
        Ref<FLabel> subtitleLabel;

    public: // - Fusion Properties -

        FUSION_PROPERTY(WeakRef<AssetBrowserGridView>, Owner);

        FUSION_PROPERTY_WRAPPER2(Text, titleLabel, Title);

        FUSION_WIDGET;
    };
    
}

#include "AssetBrowserItem.rtti.h"
