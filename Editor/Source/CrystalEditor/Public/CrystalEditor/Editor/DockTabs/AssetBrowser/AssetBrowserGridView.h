#pragma once

namespace CE::Editor
{
    class AssetBrowserItem;

    CLASS()
    class CRYSTALEDITOR_API AssetBrowserGridView : public FWrapBox
    {
        CE_CLASS(AssetBrowserGridView, FWrapBox)
    protected:

        AssetBrowserGridView();

        void Construct() override;

        void HandleEvent(FEvent* event) override;

        void OnSelectionUpdated();

        void OnBackgroundRightClicked(Vec2 globalMousePos);

        void ShowAssetContextMenu(Vec2 globalMousePos);

        FUNCTION()
        void StartRenaming();

        FUNCTION()
        void DeleteSelectedItems();

        // When user right clicks in empty space
        Ref<EditorMenuPopup> BuildNoSelectionContextMenu();

        bool BuildBasicContextMenu(EditorMenuPopup& contextMenu);

        FMenuItem& NewMenuItem();

    public: // - Public API -

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsDragEvents() const override { return true; }

        bool SupportsKeyboardFocus() const override { return true; }

        bool SupportsKeyboardEvents() const override { return true; }

        void SetCurrentDirectory(const CE::Name& directory);

        int GetSelectedItemCount();

        Array<AssetBrowserItem*> GetSelectedItems();

        // Will redraw the items
        void OnUpdate();

        void DeselectAll();

    protected: // - Internal -

        Array<AssetBrowserItem*> items;
        CE::Name currentPath;
        CE::Name folderToRename;

        Ref<EditorMenuPopup> contextMenu;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(WeakRef<AssetBrowser>, Owner);
        //FUSION_PROPERTY(Ref<AssetBrowserGridViewModel>, Model);

        FUSION_WIDGET;
        friend class AssetBrowser;
    };
    
}

#include "AssetBrowserGridView.rtti.h"
