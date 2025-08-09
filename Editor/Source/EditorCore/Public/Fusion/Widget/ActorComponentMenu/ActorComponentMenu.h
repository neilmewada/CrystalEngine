#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API ActorComponentMenu : public EditorMenuPopup
    {
        CE_CLASS(ActorComponentMenu, EditorMenuPopup)
    protected:

        ActorComponentMenu();

        void Construct() override;

    public: // - Public API -

        void Show(Ref<FButton> senderButton);

    protected: // - Internal -

        Ref<FTextInput> searchBox;
        Ref<FVerticalStack> contentBox;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "ActorComponentMenu.rtti.h"
