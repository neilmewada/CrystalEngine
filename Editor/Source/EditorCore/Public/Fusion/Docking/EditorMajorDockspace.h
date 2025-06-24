#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorMajorDockspace : public FDockspace
    {
        CE_CLASS(EditorMajorDockspace, FDockspace)
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
