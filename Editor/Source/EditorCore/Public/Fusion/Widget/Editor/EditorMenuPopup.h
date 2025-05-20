#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorMenuPopup : public FMenuPopup
    {
        CE_CLASS(EditorMenuPopup, FMenuPopup)
    public:

        // - Public API -

    protected:

        EditorMenuPopup();

        void OnBeginDestroy() override;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorMenuPopup.rtti.h"
