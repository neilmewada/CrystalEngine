#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorMajorDockspaceWindow : public FDockspaceWindow
    {
        CE_CLASS(EditorMajorDockspaceWindow, FDockspaceWindow)
    protected:

        EditorMajorDockspaceWindow();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMajorDockspaceWindow.rtti.h"
