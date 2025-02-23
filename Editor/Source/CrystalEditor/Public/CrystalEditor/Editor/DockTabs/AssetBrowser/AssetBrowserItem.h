#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API AssetBrowserItem : public FButton
    {
        CE_CLASS(AssetBrowserItem, FButton)
    protected:

        AssetBrowserItem();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "AssetBrowserItem.rtti.h"
