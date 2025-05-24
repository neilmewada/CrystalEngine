#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API AssetBrowserItem : public FSelectableButton
    {
        CE_CLASS(AssetBrowserItem, FSelectableButton)
    protected:

        AssetBrowserItem();

        void Construct() override;

        void OnSelected() override;
        void OnDeselected() override;

        void HandleEvent(FEvent* event) override;

        FUNCTION()
        void OnTextEditingFinished(FTextInput* input);

    public: // - Public API -

        void SetData(PathTreeNode* node);

        void StartEditing();

        const CE::Name& GetFullPath() const { return fullPath; }

        const CE::Name& GetItemName() const { return itemName; }

        bool IsDirectory() const { return isDirectory; }

        bool IsReadOnly() const { return isReadOnly; }

        bool IsEditing();

    protected: // - Internal -

        Ref<FCompoundWidget> titleLabelParent;
        Ref<FLabel> titleLabel;
        Ref<FTextInput> titleInput;
        Ref<FLabel> subtitleLabel;
        Ref<FStyledWidget> icon;
        Ref<FStyledWidget> iconBg;

        Ref<FVerticalStack> stackBox;

        CE::Name fullPath;
        CE::Name itemName;
        bool isDirectory = false;
        bool isReadOnly = true;

    public: // - Fusion Properties -

        FUSION_PROPERTY(WeakRef<AssetBrowserGridView>, Owner);

        FUSION_PROPERTY_WRAPPER2(Text, titleLabel, Title);

        FUSION_WIDGET;
    };
    
}

#include "AssetBrowserItem.rtti.h"
