#include "CrystalEditor.h"

namespace CE::Editor
{

    AssetBrowserItem::AssetBrowserItem()
    {
        m_ClipChildren = true;
    }

    void AssetBrowserItem::Construct()
    {
        Super::Construct();

        (*this)
            .RightClickSelects(true)
            .Width(80)
            .Height(110)
            .Style("AssetBrowserItem")
            (
                FNew(FVerticalStack)
                .Gap(2)
                .ContentHAlign(HAlign::Fill)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Fill)
                (
                    FAssignNew(FStyledWidget, iconBg)
                    .Background(Color::Black())
                    .BackgroundShape(FRoundedRectangle(5, 5, 0, 0))
                    .Height(70)
                    (
                        FAssignNew(FStyledWidget, icon)
                        .Background(FBrush("/Editor/Assets/Icons/Folder_Large"))
                        .Width(48)
                        .Height(48)
                        .HAlign(HAlign::Center)
                        .VAlign(VAlign::Center)
                    ),

                    FAssignNew(FLabel, titleLabel)
                    .Text("Asset")
                    .FontSize(9)
                    .WordWrap(FWordWrap::Normal)
                    .Foreground(Color::White())
                    .HAlign(HAlign::Left)
                    .VAlign(VAlign::Bottom)
                    .Margin(Vec4(2.5f, 0, 0, 0))
                    .Height(20),

                    FNew(FWidget)
                    .FillRatio(1.0f),

                    FAssignNew(FLabel, subtitleLabel)
                    .Text("Asset Type")
                    .FontSize(7)
                    .Foreground(Color::White().WithAlpha(0.5f))
                    .HAlign(HAlign::Left)
                    .VAlign(VAlign::Bottom)
                    .Margin(Vec4(2.5f, 0, 0, 0))
                )
            );
    }

    void AssetBrowserItem::HandleEvent(FEvent* event)
    {
        KeyModifier ctrlMod = KeyModifier::Ctrl;
        if (PlatformMisc::GetCurrentPlatform() == PlatformName::Mac)
        {
            ctrlMod = KeyModifier::Gui; // Gui is Cmd on mac
        }

        if (auto owner = m_Owner.Lock())
        {
            if (event->IsMouseEvent() && !IsInteractionDisabled() && IsVisible())
            {
                FMouseEvent* mouseEvent = static_cast<FMouseEvent*>(event);

                if (mouseEvent->type == FEventType::MousePress && mouseEvent->sender == this)
                {
                    if (mouseEvent->buttons == MouseButtonMask::Left || mouseEvent->buttons == MouseButtonMask::Right)
                    {
                        if (!EnumHasFlag(mouseEvent->keyModifiers, ctrlMod))
                        {
                            owner->DeselectAll();
                        }
                    }
                }
            }
        }

        Super::HandleEvent(event);
    }


    void AssetBrowserItem::OnSelected()
    {
        Super::OnSelected();


    }

    void AssetBrowserItem::OnDeselected()
    {
        Super::OnDeselected();

    }

    void AssetBrowserItem::SetData(PathTreeNode* node)
    {
        fullPath = nullptr;

        if (!node)
            return;

        isDirectory = node->nodeType == PathTreeNodeType::Directory;

        titleLabel->HAlign(isDirectory ? HAlign::Center : HAlign::Left);
        iconBg->Background(isDirectory ? Color::Clear() : Color::Black());
        subtitleLabel->Visible(!isDirectory);
        auto assetData = (AssetData*)node->userData;

        icon->Background(isDirectory 
            ? FBrush("/Editor/Assets/Icons/Folder_Large")
            : FBrush("/Editor/Assets/Icons/AssetIcons/DefaultFile"));

        if (!isDirectory && assetData != nullptr)
        {
            Array<String> split;
            assetData->assetClassPath.GetString().Split({ "::", "." }, split);

            ClassType* assetClass = ClassType::FindClass(assetData->assetClassPath);
            if (assetClass != nullptr)
            {
                AssetDefinition* assetDef = GetAssetDefinitionRegistry()->FindAssetDefinition(assetClass);
                if (assetDef)
                {
                    icon->Background(FBrush(assetDef->GetIconPath()));
                }
            }

            subtitleLabel->Text(split.GetLast());
        }

        Title(node->name.GetString());

        fullPath = node->GetFullPath();

        ApplyStyle();
    }

}

