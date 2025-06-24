#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorMinorDockspaceWindow : public FDockspaceWindow
    {
        CE_CLASS(EditorMinorDockspaceWindow, FDockspaceWindow)
    protected:

        EditorMinorDockspaceWindow();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMinorDockspaceWindow.rtti.h"
