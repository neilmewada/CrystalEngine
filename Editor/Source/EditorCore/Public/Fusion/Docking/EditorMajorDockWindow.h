#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorMajorDockWindow : public EditorDockWindow
    {
        CE_CLASS(EditorMajorDockWindow, EditorDockWindow)
    protected:

        EditorMajorDockWindow();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMajorDockWindow.rtti.h"
