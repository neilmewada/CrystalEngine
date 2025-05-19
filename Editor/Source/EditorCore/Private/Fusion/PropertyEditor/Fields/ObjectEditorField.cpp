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
            FNew(FButton)
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

                    FNew(FLabel)
                    .Text("[None]")
                    .WordWrap(FWordWrap::NoWrap)
                    .HAlign(HAlign::Left)
                    .VAlign(VAlign::Center)
                    .Width(92),

                    FNew(FImage)
                    .Background(assetCircle)
                    .Width(10)
                    .Height(10)
                    .VAlign(VAlign::Center)
                    .Margin(Vec4(0, 0, 10, 0))
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

    void ObjectEditorField::UpdateValue()
    {
        if (!IsBound())
        {
            return;
        }


    }
}

