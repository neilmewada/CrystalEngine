#pragma once

namespace CE::Editor
{
    class EditorToolBar;
    class EditorMenuBar;
    class EditorMinorDockspace;


    CLASS()
    class EDITORCORE_API EditorMajorDockWindow : public EditorDockWindow
    {
        CE_CLASS(EditorMajorDockWindow, EditorDockWindow)
    protected:

        EditorMajorDockWindow();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -

        Ref<EditorToolBar> toolBar;
		Ref<EditorMenuBar> menuBar;
        Ref<EditorMinorDockspace> minorDockspace;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Enabled, toolBar, ToolBarEnabled);
        FUSION_PROPERTY_WRAPPER2(Enabled, menuBar, MenuBarEnabled);

        FUSION_WIDGET;
    };
    
}

#include "EditorMajorDockWindow.rtti.h"
