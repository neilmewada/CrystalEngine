#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorMinorDockWindow : public EditorDockWindow
    {
        CE_CLASS(EditorMinorDockWindow, EditorDockWindow)
    protected:

        EditorMinorDockWindow();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMinorDockWindow.rtti.h"
