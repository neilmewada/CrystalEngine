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

        void SetOwnerEditor(Ref<EditorBase> editor);

        Ref<EditorBase> GetOwnerEditor() const
        {
            return ownerEditor.Lock();
        }

    protected: // - Internal -

        void ConstructMajorDockWindow();
        void ConstructMinorDockWindow();

        Ref<EditorToolBar> toolBar;
        Ref<EditorMenuBar> menuBar;
        Ref<EditorMinorDockspace> minorDockspace;

        WeakRef<EditorBase> ownerEditor;

        Ref<FCompoundWidget> minorDockWindowContent;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Enabled, toolBar, ToolBarEnabled);
        FUSION_PROPERTY_WRAPPER2(Enabled, menuBar, MenuBarEnabled);

        FUSION_WIDGET;
    };
    
}

#include "EditorDockWindow.rtti.h"
