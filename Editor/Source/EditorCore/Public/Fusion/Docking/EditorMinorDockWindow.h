#pragma once

namespace CE::Editor
{
    class EditorBase;

    CLASS()
    class EDITORCORE_API EditorMinorDockWindow : public EditorDockWindow
    {
        CE_CLASS(EditorMinorDockWindow, EditorDockWindow)
    protected:

        EditorMinorDockWindow();

        void Construct() override;

    public: // - Public API -

		void SetOwnerEditor(Ref<EditorBase> editor);

        Ref<EditorBase> GetOwnerEditor() const
        {
            return ownerEditor.Lock();
		}

    protected: // - Internal -

        WeakRef<EditorBase> ownerEditor;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMinorDockWindow.rtti.h"
