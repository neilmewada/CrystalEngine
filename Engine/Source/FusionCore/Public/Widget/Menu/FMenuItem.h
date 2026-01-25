#pragma once

namespace CE
{
    class FMenuPopup;

    CLASS()
    class FUSIONCORE_API FMenuItem : public FStyledWidget
    {
        CE_CLASS(FMenuItem, FStyledWidget)
    public:

        FMenuItem();

        bool IsHovered() const { return isHovered; }

        bool IsActive() const;

        bool SupportsMouseEvents() const override { return true; }

        bool SupportsFocusEvents() const override { return true; }

        void OpenSubMenu();

        FMenuPopup* GetSubMenu() const { return subMenu; }

    protected:

        void Construct() override final;

        void HandleEvent(FEvent* event) override;

        void OnAttachedToParent(FWidget* parent) override;

        FWidget* menuOwner = nullptr;

        Ref<FLabel> label;
        Ref<FImage> icon;
        Ref<FImage> arrow;

        Ref<FHorizontalStack> content;

        FMenuPopup* subMenu = nullptr;

        bool isHovered = false;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER(Foreground, label);
        FUSION_PROPERTY_WRAPPER(FontSize, label);
        FUSION_PROPERTY_WRAPPER(FontFamily, label);

        FUSION_PROPERTY_WRAPPER2(Background, icon, Icon);
        FUSION_PROPERTY_WRAPPER2(Visible, icon, IconVisible);
        FUSION_PROPERTY_WRAPPER2(Enabled, icon, IconEnabled);

        FUSION_PROPERTY_WRAPPER2(Padding, content, ContentPadding);

        FUSION_DATA_PROPERTY_WRAPPER(Text, label);

        FUSION_EVENT(FVoidEvent, OnClick);

        FMenuItem& SubMenu(FMenuPopup& subMenu);

        FUSION_WIDGET;
        friend class FMenuPopup;
        friend class FMenuBar;
    };
    
}

#include "FMenuItem.rtti.h"
