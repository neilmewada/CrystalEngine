#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API FusionDemoWindow : public FToolWindow
    {
        CE_CLASS(FusionDemoWindow, FToolWindow)
    protected:

        FusionDemoWindow();

        void Construct() override;

        void OnBeginDestroy() override;

    public: // - Public API -

        static FusionDemoWindow* Show();

    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "FusionDemoWindow.rtti.h"
