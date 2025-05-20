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

        Child(
            FNew(FHorizontalStack)
            .Gap(10)
            .ContentVAlign(VAlign::Center)
            .ContentHAlign(HAlign::Left)
            .VAlign(VAlign::Center)
            .HAlign(HAlign::Left)
            (
                FNew(FButton)
                .OnButtonClicked(FUNCTION_BINDING(this, OnButtonClicked))
                .CornerRadius(cornerRadius)
                .Height(height)
                .VAlign(VAlign::Center)
                .HAlign(HAlign::Left)
                .Style("Button.ObjectEditorField")
                .Padding(Vec4(1, 1, 1, 1))
                (
                    FNew(FHorizontalStack)
                    .Gap(5)
                    .ContentVAlign(VAlign::Fill)
                    .VAlign(VAlign::Fill)
                    .HAlign(HAlign::Fill)
                    (
                        FNew(FStyledWidget)
                        .Background(Color::Black())
                        .CornerRadius(Vec4(1, 0, 0, 1) * cornerRadius)
                        .Width(height)
                        .Height(height)
                        .Margin(Vec4(0, 0, 7.5f, 0)),

                        FNew(FVerticalStack)
                        .Gap(2)
                        .ClipChildren(true)
                        .HAlign(HAlign::Left)
                        .VAlign(VAlign::Center)
                        .Width(92)
                        (
                            FAssignNew(FLabel, valueLabel)
                            .Text("[None]")
                            .WordWrap(FWordWrap::NoWrap)
                            .HAlign(HAlign::Left),

                            FAssignNew(FLabel, pathLabel)
                            .Text("")
                            .FontSize(8)
                            .Foreground(Color::White().WithAlpha(0.5f))
                            .Enabled(false)
                        ),

                        FNew(FImage)
                        .Background(assetCircle)
                        .Width(10)
                        .Height(10)
                        .VAlign(VAlign::Center)
                        .Margin(Vec4(0, 0, 10, 0))
                    )
                ),

                FNew(FImageButton)
                .Image(FBrush("/Engine/Resources/Icons/CrossIcon"))
                .OnClicked([this]
                {
                    SelectAsset(nullptr);
                })
                .Width(12)
                .Height(12)
                .Padding(Vec4(1, 1, 1, 1) * 2.5f)
                .Style("Button.Icon")
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
            pathLabel->Enabled(false);
        }
        else
        {
            pathLabel->Enabled(true);

            Ref<Bundle> bundle = value->GetBundle();
            if (bundle && !bundle->IsTransient())
            {
                valueLabel->Text(bundle->GetName().GetString());
                pathLabel->Text(bundle->GetBundlePath().GetString());
            }
            else
            {
                valueLabel->Text(value->GetName().GetString());
                pathLabel->Text(value->GetPathInBundle().GetString());
            }
        }
    }
}

