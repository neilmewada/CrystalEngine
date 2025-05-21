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
                    FNew(FStyledWidget) // Thumbnail
                    .Background(Color::Black())
                    .CornerRadius(Vec4(1, 1, 1, 1) * cornerRadius)
                    .Width(height)
                    .Height(height)
                    .Margin(Vec4(0, 0, 7.5f, 0)),

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
                            .HAlign(HAlign::Fill)
                            .VAlign(VAlign::Fill)
                            (
                                FAssignNew(FLabel, valueLabel)
                                .Text("[None]")
                                .WordWrap(FWordWrap::NoWrap)
                                .FontSize(10)
                                .HAlign(HAlign::Left)
                                .FillRatio(1.0f),

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
            }
        }

        curValue = initialValue;
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

                curObjectFullPath = bundle->GetBundlePath();
            }
            else
            {
                valueLabel->Text(value->GetName().GetString());
            }
        }
    }
}

