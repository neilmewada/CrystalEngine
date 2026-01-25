#pragma once

namespace CE::Editor
{
    class EditorBase;

    CLASS()
    class EDITORCORE_API EditorMajorDockspaceWindow : public FDockspaceWindow
    {
        CE_CLASS(EditorMajorDockspaceWindow, FDockspaceWindow)
    protected:

        EditorMajorDockspaceWindow();

        void Construct() override;

    public: // - Public API -

        bool OpenEditor(Ref<Object> targetObject, Ref<Bundle> bundle = nullptr);

        bool OpenEditor(const CE::Name& assetPath);

        bool SelectActiveEditor(Ref<EditorBase> editor);

    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMajorDockspaceWindow.rtti.h"
