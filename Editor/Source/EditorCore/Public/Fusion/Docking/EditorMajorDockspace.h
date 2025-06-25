#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorMajorDockspace : public EditorDockspace
    {
        CE_CLASS(EditorMajorDockspace, EditorDockspace)
    protected:

        EditorMajorDockspace();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMajorDockspace.rtti.h"
