#pragma once

namespace CE::Editor
{
    class EditorDockTab;
    class EditorDockTabItem;
    class EditorBase;

    CLASS()
    class EDITORCORE_API EditorDockspace : public FDockspace
    {
        CE_CLASS(EditorDockspace, FDockspace)
    public:

        // - Public API -

        

    protected:

        EditorDockspace();

        void Construct() override;

        void OnBeginDestroy() override;

        Array<WeakRef<EditorBase>> openedEditors;

    public: // - Fusion Properties - 

        FUSION_WIDGET;
        friend class EditorDockspaceStyle;
    };
    
}

#include "EditorDockspace.rtti.h"
