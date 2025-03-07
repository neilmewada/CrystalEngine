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

    public: // - Public API -

        void SetData(PathTreeNode* node);

        CE::Name GetFullPath() const { return fullPath; }

        bool IsDirectory() const { return isDirectory; }

    protected: // - Internal -

        Ref<FLabel> titleLabel;
        Ref<FLabel> subtitleLabel;
        Ref<FStyledWidget> icon;
        Ref<FStyledWidget> iconBg;

        CE::Name fullPath;
        bool isDirectory = false;

    public: // - Fusion Properties -

        FUSION_PROPERTY(WeakRef<AssetBrowserGridView>, Owner);

        FUSION_PROPERTY_WRAPPER2(Text, titleLabel, Title);

        FUSION_WIDGET;
    };
    
}

#include "AssetBrowserItem.rtti.h"
