#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API AssetBrowser : public EditorMinorDockTab
    {
        CE_CLASS(AssetBrowser, EditorMinorDockTab)
    protected:

        AssetBrowser();

        void Construct() override;

    public: // - Public API -



    protected: // - Internal -


        FUSION_WIDGET;
    };
    
}

#include "AssetBrowser.rtti.h"
