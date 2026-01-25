#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorMinorDockspace : public EditorDockspace
    {
        CE_CLASS(EditorMinorDockspace, EditorDockspace)
    protected:

        EditorMinorDockspace();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMinorDockspace.rtti.h"
