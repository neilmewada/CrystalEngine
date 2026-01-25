#include "EditorCore.h"

namespace CE::Editor
{

    ObjectEditorField::ObjectEditorField()
    {

    }

    void ObjectEditorField::Construct()
    {
        Super::Construct();

        constexpr f32 height = 50;
        constexpr f32 cornerRadius = 5;

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        FBrush assetCircle = FBrush("/Editor/Assets/Icons/DoubleCircle", Color::RGBHex(0xd9d9d9));

        FBrush useAsset = FBrush("/Editor/Assets/Icons/SelectFile");
        FBrush showAsset = FBrush("/Editor/Assets/Icons/FolderSearch");
        FBrush crossIcon = FBrush("/Editor/Assets/Icons/Cross");

        Child(
            FNew(FHorizontalStack)
            .Gap(10)
            .ContentVAlign(VAlign::Center)
            .ContentHAlign(HAlign::Left)
            .VAlign(VAlign::Center)
            .HAlign(HAlign::Left)
            (
                FNew(FHorizontalStack)
                .Gap(5)
                .ContentVAlign(VAlign::Fill)
                .VAlign(VAlign::Fill)
                .HAlign(HAlign::Fill)
                (
                    FNew(FOverlayStack)
                    .ContentHAlign(HAlign::Center)
                    .ContentVAlign(VAlign::Center)
                    .Width(height)
                    .Height(height)
                    .Margin(Vec4(0, 0, 7.5f, 0))
                    (
                        FNew(FStyledWidget)
                        .Background(Colors::Black)
                        .CornerRadius(Vec4(1, 1, 1, 1) * cornerRadius)
                        .Width(height)
                        .Height(height),

                        FAssignNew(FImage, thumbnail)
                        .Background(Colors::Cyan)
                        .Width(38)
                        .Height(38),

                        FAssignNew(FStyledWidget, colorTag)
                        .Background(Colors::Clear)
                        .CornerRadius(Vec4(0, 0, 1, 1) * 2.0f)
                        .Height(2.0f)
                        .VAlign(VAlign::Bottom)
                        .HAlign(HAlign::Fill)
                    ),

                    FNew(FVerticalStack)
                    .Gap(2)
                    .ClipChildren(false)
                    .HAlign(HAlign::Left)
                    .VAlign(VAlign::Top)
                    .Width(140)
                    .Margin(Vec4(0, 0, 5, 0))
                    (
                        FNew(FButton)
                        .OnButtonClicked(FUNCTION_BINDING(this, OnButtonClicked))
                        .CornerRadius(Vec4(1, 1, 1, 1) * cornerRadius)
                        .HAlign(HAlign::Fill)
                        .Padding(Vec4(1, 1, 1, 1) * 4.0f)
                        .Style("Button.ObjectEditorField")
                        (
                            FNew(FHorizontalStack)
                            .Gap(2.5f)
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Fill)
                            (
                                FNew(FCompoundWidget)
                                .ClipChildren(true)
                                .FillRatio(1.0f)
                                (
                                    FAssignNew(FLabel, valueLabel)
                                    .Text("[None]")
                                    .WordWrap(FWordWrap::NoWrap)
                                    .FontSize(fontSize)
                                    .HAlign(HAlign::Fill)
                                ),

                                FNew(FImage)
                                .Background(assetCircle)
                                .Width(10)
                                .Height(10)
                                .VAlign(VAlign::Center)
                            )
                        ),

                        FNew(FWidget)
                        .FillRatio(1.0f),

                        FNew(FHorizontalStack)
                        .Gap(7.0f)
                        .HAlign(HAlign::Left)
                        (
                            FNew(FImageButton)
                            .Image(useAsset)
                            .OnClicked([this]
                            {
                                UseSelectedAsset();
                            })
                            .Width(16)
                            .Height(16)
                            .Padding(Vec4(1, 1, 1, 1) * 2.5f)
                            .VAlign(VAlign::Center)
                            .Style("Button.Icon"),

                            FNew(FImageButton)
                            .Image(showAsset)
                            .OnClicked([this]
                            {
                                BrowseToAsset();
                            })
                            .Width(16)
                            .Height(16)
                            .Padding(Vec4(1, 1, 1, 1) * 2.5f)
                            .VAlign(VAlign::Center)
                            .Style("Button.Icon"),

                            FNew(FImageButton)
                            .Image(crossIcon)
                            .OnClicked([this]
                            {
                                SelectAsset(nullptr);
                            })
                            .Width(16)
                            .Height(16)
                            .Padding(Vec4(1, 1, 1, 1) * 2.5f)
                            .Style("Button.Icon")
                        )
                    )
                )
            )
        );
    }

    void ObjectEditorField::OnBind()
    {
        Super::OnBind();

        if (!registered)
        {
            registered = true;

            AssetRegistry::Get()->AddRegistryListener(this);
			ThumbnailSystem::Get()->AddThumbnailListener(this);
        }

        TypeInfo* typeInfo = GetTypeInfo(fieldDeclId);
        if (!typeInfo || !typeInfo->IsClass())
            return;

		ClassType* classType = (ClassType*)typeInfo;

        if (AssetDefinition* assetDef = AssetDefinitionRegistry::Get()->FindAssetDefinition(classType))
        {
			colorTag->Background(assetDef->GetColorTag());
        }
    }

    void ObjectEditorField::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

        if (registered)
        {
            registered = false;

            AssetRegistry::Get()->RemoveRegistryListener(this);
			ThumbnailSystem::Get()->RemoveThumbnailListener(this);
        }
    }

    void ObjectEditorField::OnAssetRenamed(Uuid bundleUuid, const CE::Name& oldName, const CE::Name& newName, const CE::Name& newPath)
    {
        if (Ref<Object> object = curValue.Lock())
        {
            if (Ref<Bundle> bundle = object->GetBundle())
            {
                if (bundle->GetUuid() == bundleUuid)
                {
                    UpdateValue();
                }
            }
        }
    }

    void ObjectEditorField::OnAssetDeleted(const CE::Name& bundlePath)
    {
        if (Ref<Object> object = curValue.Lock())
        {
            if (Ref<Bundle> bundle = object->GetBundle())
            {
                if (bundle->GetBundlePath() == bundlePath)
                {
                    UpdateValue();
                }
            }
        }
    }

    void ObjectEditorField::OnDirectoryRenamed(const CE::Name& oldPath, const CE::Name& newPath)
    {
        if (Ref<Object> object = curValue.Lock())
        {
            if (Ref<Bundle> bundle = object->GetBundle())
            {
                String bundlePath = bundle->GetBundlePath().GetCString();
                if (bundlePath.Contains(newPath.GetString()))
                {
                    UpdateValue();
                }
            }
        }
    }

    void ObjectEditorField::OnThumbnailsUpdated(const Array<CE::Name>& assetPaths)
    {
        UpdateValue();
    }

    bool ObjectEditorField::CanBind(FieldType* field)
    {
        TypeInfo* fieldDeclType = field->GetDeclarationType();
        if (!fieldDeclType)
            return false;
        return fieldDeclType->IsClass();
    }

    bool ObjectEditorField::CanBind(TypeId boundTypeId, TypeId underlyingTypeId)
    {
        TypeInfo* type = GetTypeInfo(boundTypeId);
        return type != nullptr && type->IsClass();
    }

    void ObjectEditorField::OnButtonClicked(FButton* button, Vec2 mousePos)
    {
        TypeInfo* typeInfo = GetTypeInfo(fieldDeclId);
        if (!typeInfo || !typeInfo->IsClass())
            return;

        Ref<AssetSelectionPopup> popup = CreateObject<AssetSelectionPopup>(this, "AssetSelector");
        popup->SetAssetClass((ClassType*)typeInfo);
        popup->SetCurrentValue(curValue);

        GetContext()->PushLocalPopup(popup.Get(),
            button->GetGlobalPosition() + button->GetComputedSize() * Vec2(0, 1),
            Vec2(-1, 340), button->GetComputedSize());

        WeakRef<ObjectEditorField> thisRef = this;

        popup->OnClosed([thisRef](FPopup* popup)
        {
            popup->QueueDestroy();
        });
        
        popup->OnAssetSelected([this] (AssetData* assetData)
        {
            SelectAsset(assetData);
        });

        popup->StartEditingSearchBox();
    }

    void ObjectEditorField::UseSelectedAsset()
    {
        if (Ref<ObjectEditor> objectEditor = m_ObjectEditorOwner.Lock())
        {
            Array<CE::Name> assetPaths = objectEditor->FetchSelectedAssetPaths();

            for (const CE::Name& assetPath : assetPaths)
            {
                AssetData* assetData = AssetRegistry::Get()->GetPrimaryAssetByPath(assetPath);
                if (!assetData)
                    continue;

                ClassType* assetClass = ClassType::FindClass(assetData->assetClassTypeName);
                if (!assetClass)
                    continue;

                if (assetClass->IsSubclassOf(fieldDeclId))
                {
                    SelectAsset(assetData);
                    break;
                }
            }
        }
    }

    void ObjectEditorField::BrowseToAsset()
    {
        if (Ref<ObjectEditor> objectEditor = m_ObjectEditorOwner.Lock())
        {
            objectEditor->BrowseToAssetInAssetBrowser(curObjectFullPath);
        }
    }

    void ObjectEditorField::SelectAsset(AssetData* assetData)
    {
        if (!IsBound())
            return;

        Ref<Object> target = targets[0].Lock();
        if (target.IsNull())
            return;

        Ptr<FieldType> field = nullptr;
        void* instance = nullptr;

        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
            field, instance);

        if (!success || !field->IsObjectField())
            return;

        Ref<Object> asset = assetData != nullptr
            ? AssetManager::Get()->LoadAssetAtPath(assetData->bundlePath)
            : nullptr;

        WeakRef<Object> initialValue = nullptr;

        if (field->IsStrongRefCounted())
        {
            initialValue = field->GetFieldValue<Ref<Object>>(instance);

            if (auto history = m_History.Lock())
            {
                history->PerformObjectOperation("Edit Object Field", target, relativeFieldPath,
                    initialValue, asset);
            }
            else
            {
                field->SetFieldValue<Ref<Object>>(instance, asset);
                target->OnFieldEdited(relativeFieldPath);
            }
        }
        else if (field->IsWeakRefCounted())
        {
            initialValue = field->GetFieldValue<WeakRef<Object>>(instance);

            if (auto history = m_History.Lock())
            {
                history->PerformObjectOperation("Edit Object Field", target, relativeFieldPath,
                    initialValue, asset);
            }
            else
            {
                field->SetFieldValue<WeakRef<Object>>(instance, asset);
                target->OnFieldEdited(relativeFieldPath);
            }
        }
        else
        {
            initialValue = field->GetFieldValue<Object*>(instance);

            if (auto history = m_History.Lock())
            {
                history->PerformObjectOperation("Edit Object Field", target, relativeFieldPath,
                    initialValue, asset);
            }
            else
            {
                field->SetFieldValue<Object*>(instance, asset.Get());
                target->OnFieldEdited(relativeFieldPath);
            }
        }

        curValue = asset;
    }

    void ObjectEditorField::UpdateValue()
    {
        if (!IsBound())
            return;

        Ref<Object> target = targets[0].Lock();
        if (target.IsNull())
            return;

        Ptr<FieldType> field = nullptr;
        void* instance = nullptr;

        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
            field, instance);

        if (!success || !field->IsObjectField())
            return;

        Ref<Object> value = nullptr;

        if (field->IsStrongRefCounted())
        {
            value = field->GetFieldValue<Ref<Object>>(instance);
        }
        else if (field->IsWeakRefCounted())
        {
            value = field->GetFieldValue<WeakRef<Object>>(instance).Lock();
        }
        else
        {
            value = field->GetFieldValue<Object*>(instance);
        }

        curValue = value;

        thumbnail->Background(Colors::Clear);

        if (value == nullptr)
        {
            valueLabel->Text("[None]");
            curObjectFullPath = nullptr;
        }
        else
        {
        	Ref<Bundle> bundle = value->GetBundle();
            if (bundle && !bundle->IsTransient())
            {
                valueLabel->Text(bundle->GetName().GetString());

            	CE::Name bundlePath = bundle->GetBundlePath();
				CE::Name thumbnailPath = ThumbnailSystem::GetThumbnailPath(bundlePath);
				IO::Path thumbnailAbsolutePath = Bundle::GetAbsoluteBundlePath(thumbnailPath);
                if (thumbnailAbsolutePath.Exists())
                {
                    FBrush thumbnailBrush = FBrush(thumbnailPath);
                    thumbnail->Background(thumbnailBrush);
                }
                else
                {
                    ClassType* assetClass = value->GetClass();
                    if (AssetDefinition* assetDef = AssetDefinitionRegistry::Get()->FindAssetDefinition(assetClass))
                    {
                        thumbnail->Background(FBrush(assetDef->GetIconPath()));
                    }
				}

                curObjectFullPath = bundle->GetBundlePath();
            }
            else
            {
                valueLabel->Text(value->GetName().GetString());
            }
        }
    }
}

