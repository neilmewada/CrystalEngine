#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API AddComponentMenu : public EditorMenuPopup
    {
        CE_CLASS(AddComponentMenu, EditorMenuPopup)
    protected:

        AddComponentMenu();

        void Construct() override;

        void OnPopupClosed() override;

    public: // - Public API -

        void Show(Ref<FButton> senderButton);

    protected: // - Internal -

        Ref<FTextInput> searchBox;

        Ref<AddComponentTreeView> treeView;
        Ref<AddComponentTreeViewModel> treeViewModel;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(ScriptDelegate<void(ClassType*)>, OnComponentClassSelected);

        FUSION_WIDGET;
    };
    
}

#include "AddComponentMenu.rtti.h"
