#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API AssetSelectionPopup : public EditorMenuPopup
    {
        CE_CLASS(AssetSelectionPopup, EditorMenuPopup)
    protected:

        AssetSelectionPopup();

        void OnBeginDestroy() override;

        void Construct() override;

        void HandleEvent(FEvent* event) override;

    public: // - Public API -

        bool SupportsKeyboardEvents() const override { return true; }
        bool SupportsKeyboardFocus() const override { return true; }

        void SetAssetClass(ClassType* assetClass);

        void SetCurrentValue(WeakRef<Object> curValue);

        void StartEditingSearchBox();

        FMenuItem& NewMenuItem();

    protected: // - Internal -

        Ref<AssetSelectionListViewModel> model;
        Ref<AssetSelectionListView> listView;
        Ref<FTextInput> searchBox;

        WeakRef<Object> curValue;

    public: // - Fusion Properties - 

        FUSION_EVENT(ScriptEvent<void(AssetData*)>, OnAssetSelected);

        FUSION_WIDGET;
    };
    
}

#include "AssetSelectionPopup.rtti.h"
