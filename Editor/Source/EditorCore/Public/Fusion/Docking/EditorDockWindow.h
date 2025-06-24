#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorDockWindow : public FDockWindow
    {
        CE_CLASS(EditorDockWindow, FDockWindow)
    protected:

        EditorDockWindow();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorDockWindow.rtti.h"
