#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API MaterialDetailsTab : public EditorMinorDockWindow
    {
        CE_CLASS(MaterialDetailsTab, EditorMinorDockWindow)
    protected:

        MaterialDetailsTab();

        void Construct() override;

    public: // - Public API -

        void SetupEditor(Ref<CE::Material> material);

    protected: // - Internal -

        Ref<FStyledWidget> editorContainer;

        Ref<ObjectEditor> editor = nullptr;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "MaterialDetailsTab.rtti.h"
