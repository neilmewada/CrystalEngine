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
            FAssignNew(FVerticalStack, stackBox)
            .Gap(2)
            .ContentHAlign(HAlign::Fill)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
                FAssignNew(FStyledWidget, iconBg)
                .Background(Color::Black())
                .BackgroundShape(FRoundedRectangle(5, 5, 0, 0))
                .Height(68)
                (
                    FAssignNew(FStyledWidget, icon)
                    .Background(FBrush("/Editor/Assets/Icons/Folder_Large"))
                    .Width(48)
                    .Height(48)
                    .HAlign(HAlign::Center)
                    .VAlign(VAlign::Center)
                ),

                FAssignNew(FStyledWidget, colorTag)
                .Background(Color::Cyan())
                .Height(2.5f),

                FAssignNew(FCompoundWidget, titleLabelParent)
                .ClipChildren(true)
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Bottom)
                .Height(20)
                (
                    FAssignNew(FLabel, titleLabel)
                    .Text("Asset")
                    .FontSize(9)
                    .WordWrap(FWordWrap::Normal)
                    .Foreground(Color::White())
                    .HAlign(HAlign::Left)
                    .VAlign(VAlign::Bottom)
                    .Margin(Vec4(2.5f, 0, 2.5f, 0))
                    .Height(20)
                ),

                FAssignNew(FTextInput, titleInput)
                .Text("Asset")
                .FontSize(9)
                .OnTextEditingFinished(FUNCTION_BINDING(this, OnTextEditingFinished))
                .Validator([this](const String& in) -> bool
                {
                    for (int i = 0; i < in.GetLength(); ++i)
                    {
                        char c = in[i];
                        if (!String::IsAlphabet(c) && !String::IsNumeric(c) && c != '_' && c != '-')
                        {
                            return false;
                        }
                    }

                    return true;
                })
                .HAlign(HAlign::Fill)
                .VAlign(VAlign::Bottom)
                .Enabled(false),

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

        if (titleInput->IsEditing())
        {
            titleInput->StopEditing();
        }
    }

    void AssetBrowserItem::SetData(PathTreeNode* node)
    {
        fullPath = nullptr;

        if (!node)
            return;

        isDirectory = node->nodeType == PathTreeNodeType::Directory;

        titleLabel->HAlign(isDirectory ? HAlign::Center : HAlign::Left);
        titleLabel->Margin(isDirectory ? Vec4() : Vec4(2.5f, 0, 2.5f, 0));

        colorTag->Visible(!isDirectory);
        iconBg->Background(isDirectory ? Color::Clear() : Color::Black());
        subtitleLabel->Visible(!isDirectory);
        auto assetData = (AssetData*)node->userData;

        icon->Background(isDirectory 
            ? FBrush("/Editor/Assets/Icons/Folder_Large")
            : FBrush("/Editor/Assets/Icons/AssetIcons/DefaultFile"));

        if (!isDirectory && assetData != nullptr)
        {
            Array<String> split;
            assetData->assetClassTypeName.GetString().Split({ "::", "." }, split);

            CE::Name thumbnailPath = ThumbnailSystem::GetThumbnailPath(assetData->bundlePath);
			IO::Path thumbnailAbsolutePath = Bundle::GetAbsoluteBundlePath(thumbnailPath);
            bool thumbnailFound = false;
            if (thumbnailAbsolutePath.Exists())
            {
				thumbnailFound = true;
                FBrush thumbnail = FBrush(thumbnailPath);
                icon->Background(thumbnail);
                icon->Width(56);
				icon->Height(56);
            }

            ClassType* assetClass = ClassType::FindClass(assetData->assetClassTypeName);
            if (assetClass != nullptr)
            {
                AssetDefinition* assetDef = GetAssetDefinitionRegistry()->FindAssetDefinition(assetClass);
                if (assetDef)
                {
                    if (!thumbnailFound)
                    {
	                    icon->Background(FBrush(assetDef->GetIconPath()));
                    }
                    colorTag->Background(assetDef->GetColorTag());
                }
            }

            subtitleLabel->Text(split.GetLast());
        }

        Title(node->name.GetString());

        fullPath = node->GetFullPath();
        itemName = node->name;

        isReadOnly = !fullPath.GetString().StartsWith("/Game/Assets");

        ApplyStyle();
    }

    void AssetBrowserItem::StartEditing()
    {
        (*titleInput)
        .Text(titleLabel->Text())
        .Enabled(true);

        titleLabelParent->Enabled(false);

        titleInput->StartEditing(true, true);
        titleInput->Focus();
    }

    bool AssetBrowserItem::IsEditing()
    {
        return titleInput->Enabled() && titleInput->IsEditing();
    }

    void AssetBrowserItem::OnTextEditingFinished(FTextInput*)
    {
        titleLabelParent->Enabled(true);
        titleInput->Enabled(false);

        Ref<AssetBrowserGridView> gridView = m_Owner.Lock();
        if (!gridView)
            return;

        Ref<AssetBrowser> assetBrowser = gridView->Owner().Lock();
        if (!assetBrowser)
            return;

        bool success = false;

        if (isDirectory)
        {
            success = assetBrowser->RenameDirectory(fullPath, titleInput->Text());
        }
        else // Asset
        {
            success = assetBrowser->RenameAsset(fullPath, titleInput->Text());
        }

        if (!success)
            return;

        titleLabel->Text(titleInput->Text());
    }

}

