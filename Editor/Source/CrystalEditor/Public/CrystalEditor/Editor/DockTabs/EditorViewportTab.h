#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API EditorViewportTab : public EditorMinorDockWindow
    {
        CE_CLASS(EditorViewportTab, EditorMinorDockWindow)
    public:

        // - Public API -

        EditorViewport* GetViewport() const { return viewport; }

    protected:

        EditorViewportTab();

        void Construct() override;

        FIELD()
        EditorViewport* viewport = nullptr;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "EditorViewportTab.rtti.h"
