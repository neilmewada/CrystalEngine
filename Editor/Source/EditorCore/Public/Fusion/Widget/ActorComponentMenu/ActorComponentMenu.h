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

        void OnPopupClosed() override;

    public: // - Public API -

        void Show(Ref<FButton> senderButton);

    protected: // - Internal -

        Ref<FTextInput> searchBox;

        Ref<ActorComponentTreeView> treeView;
        Ref<ActorComponentTreeViewModel> treeViewModel;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "ActorComponentMenu.rtti.h"
