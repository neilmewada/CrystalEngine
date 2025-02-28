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

    public: // - Public API -


    protected: // - Internal -

        Ref<FLabel> titleLabel;

    public: // - Fusion Properties -

        FUSION_PROPERTY_WRAPPER2(Text, titleLabel, Title);

        FUSION_WIDGET;
    };
    
}

#include "AssetBrowserItem.rtti.h"
