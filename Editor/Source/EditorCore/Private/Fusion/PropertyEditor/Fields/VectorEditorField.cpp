#include "EditorCore.h"

namespace CE::Editor
{

    VectorEditorField::VectorEditorField()
    {

    }

    void VectorEditorField::Construct()
    {
        Super::Construct();

        Child(
            FNew(FHorizontalStack)
            .Gap(1)
            .ContentVAlign(VAlign::Fill)
            .ContentHAlign(HAlign::Fill)
            (
                FAssignNew(NumericEditorField, fieldX)
                .OnTextEdited(FUNCTION_BINDING(this, OnTextEdited))
                .OnTextEditingFinished(FUNCTION_BINDING(this, OnTextEditingFinished))
                .OnBeginEditing(FUNCTION_BINDING(this, OnBeginEditing))
                .OnEndEditing(FUNCTION_BINDING(this, OnEndEditing))
                .ColorTagVisible(true)
                .ColorTag(Colors::Red)
                .VAlign(VAlign::Fill)
                .FillRatio(1.0f),

                FAssignNew(NumericEditorField, fieldY)
                .OnTextEdited(FUNCTION_BINDING(this, OnTextEdited))
                .OnTextEditingFinished(FUNCTION_BINDING(this, OnTextEditingFinished))
                .OnBeginEditing(FUNCTION_BINDING(this, OnBeginEditing))
                .OnEndEditing(FUNCTION_BINDING(this, OnEndEditing))
                .ColorTagVisible(true)
                .ColorTag(Colors::Green)
                .VAlign(VAlign::Fill)
                .FillRatio(1.0f),

                FAssignNew(NumericEditorField, fieldZ)
                .OnTextEdited(FUNCTION_BINDING(this, OnTextEdited))
                .OnTextEditingFinished(FUNCTION_BINDING(this, OnTextEditingFinished))
                .OnBeginEditing(FUNCTION_BINDING(this, OnBeginEditing))
                .OnEndEditing(FUNCTION_BINDING(this, OnEndEditing))
                .ColorTagVisible(true)
                .ColorTag(Colors::Blue)
                .VAlign(VAlign::Fill)
                .FillRatio(1.0f),

                FAssignNew(NumericEditorField, fieldW)
                .OnTextEdited(FUNCTION_BINDING(this, OnTextEdited))
                .OnTextEditingFinished(FUNCTION_BINDING(this, OnTextEditingFinished))
                .OnBeginEditing(FUNCTION_BINDING(this, OnBeginEditing))
                .OnEndEditing(FUNCTION_BINDING(this, OnEndEditing))
                .ColorTagVisible(true)
                .ColorTag(Colors::White)
                .VAlign(VAlign::Fill)
                .FillRatio(1.0f)
            )
        );

        VectorType<Vec3>();
    }

    bool VectorEditorField::CanBind(FieldType* field)
    {
        return field->GetDeclarationType()->IsVectorType();
    }

    bool VectorEditorField::CanBind(TypeId boundTypeId, TypeId underlyingTypeId)
    {
        static HashSet<TypeId> vectorTypes = {
            TYPEID(Vec2), TYPEID(Vec2i),
            TYPEID(Vec3), TYPEID(Vec3i),
            TYPEID(Vec4), TYPEID(Vec4i),
        };

        return vectorTypes.Exists(boundTypeId);
    }

    void VectorEditorField::UpdateValue()
    {
        if (!IsBound())
            return;

        thread_local HashSet floatVectors = { TYPEID(Vec2), TYPEID(Vec3), TYPEID(Vec4) };
        thread_local HashSet intVectors = { TYPEID(Vec2i), TYPEID(Vec3i), TYPEID(Vec4i) };

        Ref<Object> target = targets[0].Lock();
        if (target.IsNull())
            return;

        Ptr<FieldType> field;
        void* instance = nullptr;

        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target, field, instance);
        if (!success)
            return;

        TypeId fieldDeclId = field->GetDeclarationTypeId();

        if (fieldDeclId == TYPEID(Vec2))
        {
            Vec2 value = field->GetFieldValue<Vec2>(instance);
            if (!fieldX->IsEditing())
				fieldX->Text(String::Format("{}", value.x));
            if (!fieldY->IsEditing())
				fieldY->Text(String::Format("{}", value.y));
        }
        else if (fieldDeclId == TYPEID(Vec2i))
        {
            Vec2i value = field->GetFieldValue<Vec2i>(instance);
            if (!fieldX->IsEditing())
				fieldX->Text(String::Format("{}", value.x));
            if (!fieldY->IsEditing())
        		fieldY->Text(String::Format("{}", value.y));
        }
        else if (fieldDeclId == TYPEID(Vec3))
        {
            Vec3 value = field->GetFieldValue<Vec3>(instance);
            if (!fieldX->IsEditing())
				fieldX->Text(String::Format("{}", value.x));
            if (!fieldY->IsEditing())
				fieldY->Text(String::Format("{}", value.y));
            if (!fieldZ->IsEditing())
				fieldZ->Text(String::Format("{}", value.z));
        }
        else if (fieldDeclId == TYPEID(Vec3i))
        {
            Vec3i value = field->GetFieldValue<Vec3i>(instance);
            if (!fieldX->IsEditing())
				fieldX->Text(String::Format("{}", value.x));
            if (!fieldY->IsEditing())
        		fieldY->Text(String::Format("{}", value.y));
            if (!fieldZ->IsEditing())
				fieldZ->Text(String::Format("{}", value.z));
        }
        else if (fieldDeclId == TYPEID(Vec4))
        {
            Vec4 value = field->GetFieldValue<Vec4>(instance);
            if (!fieldX->IsEditing())
				fieldX->Text(String::Format("{}", value.x));
            if (!fieldY->IsEditing())
				fieldY->Text(String::Format("{}", value.y));
            if (!fieldZ->IsEditing())
				fieldZ->Text(String::Format("{}", value.z));
            if (!fieldW->IsEditing())
				fieldW->Text(String::Format("{}", value.w));
        }
        else if (fieldDeclId == TYPEID(Vec4i))
        {
            Vec4i value = field->GetFieldValue<Vec4i>(instance);
            if (!fieldX->IsEditing())
				fieldX->Text(String::Format("{}", value.x));
            if (!fieldY->IsEditing())
				fieldY->Text(String::Format("{}", value.y));
            if (!fieldZ->IsEditing())
        		fieldZ->Text(String::Format("{}", value.z));
            if (!fieldW->IsEditing())
				fieldW->Text(String::Format("{}", value.w));
        }
    }

    void VectorEditorField::OnTextEdited(NumericEditorField*)
    {
        if (!IsBound())
            return;

        SetFieldValue();
    }

    void VectorEditorField::OnTextEditingFinished(NumericEditorField*)
    {
        if (!IsBound())
            return;

        SetFieldValue();
    }

    void VectorEditorField::OnEndEditing(NumericEditorField* input)
    {
        if (!IsBound())
            return;

        Ref<Object> target = targets[0].Lock();
        if (target.IsNull())
            return;

        Ptr<FieldType> field;
        void* instance = nullptr;

        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target, field, instance);
        if (!success)
            return;

        TypeId fieldDeclId = field->GetDeclarationTypeId();
        CE::Name relativePath = relativeFieldPath;
        WeakRef<Object> targetRef = target;

        if (fieldDeclId == TYPEID(Vec2))
        {
            Vec2 value;
            Vec2 initialValue = this->initialValue;

            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y))
            {
                if (value != initialValue)
                {
                    if (auto history = m_History.Lock())
                    {
                        history->PerformOperation("Edit Vector Field", target, relativePath, initialValue, value);
                    }
                    else
                    {
                        field->SetFieldValue(instance, value);
                        target->OnFieldEdited(field->GetName());
                    }
                }
            }
        }
        else if (fieldDeclId == TYPEID(Vec2i))
        {
            Vec2i value;
            Vec2i initialValue = this->initialValue.ToVec4i();

            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y))
            {
                if (value != initialValue)
                {
                    if (auto history = m_History.Lock())
                    {
                        history->PerformOperation("Edit Vector Field", target, relativePath, initialValue, value);
                    }
                    else
                    {
                        field->SetFieldValue(instance, value);
                        target->OnFieldEdited(field->GetName());
                    }
                }
            }
        }
        else if (fieldDeclId == TYPEID(Vec3))
        {
            Vec3 value;
            Vec3 initialValue = this->initialValue;

            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            const String& z = fieldZ->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y) &&
                String::TryParse(z, value.z))
            {
                if (value != initialValue)
                {
                    if (auto history = m_History.Lock())
                    {
                        history->PerformOperation("Edit Vector Field", target, relativePath, initialValue, value);
                    }
                    else
                    {
                        field->SetFieldValue(instance, value);
                        target->OnFieldEdited(field->GetName());
                    }
                }
            }
        }
        else if (fieldDeclId == TYPEID(Vec3i))
        {
            Vec3i value;
            Vec3i initialValue = this->initialValue.ToVec4i();

            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            const String& z = fieldZ->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y) &&
                String::TryParse(z, value.z))
            {
                if (value != initialValue)
                {
                    if (auto history = m_History.Lock())
                    {
                        history->PerformOperation("Edit Vector Field", target, relativePath, initialValue, value);
                    }
                    else
                    {
                        field->SetFieldValue(instance, value);
                        target->OnFieldEdited(field->GetName());
                    }
                }
            }
        }
        else if (fieldDeclId == TYPEID(Vec4))
        {
            Vec4 value;
            Vec4 initialValue = this->initialValue;

            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            const String& z = fieldZ->Text();
            const String& w = fieldW->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y) &&
                String::TryParse(z, value.z) &&
                String::TryParse(w, value.w))
            {
                if (value != initialValue)
                {
                    if (auto history = m_History.Lock())
                    {
                        history->PerformOperation("Edit Vector Field", target, relativePath, initialValue, value);
                    }
                    else
                    {
                        field->SetFieldValue(instance, value);
                        target->OnFieldEdited(field->GetName());
                    }
                }
            }
        }
        else if (fieldDeclId == TYPEID(Vec4i))
        {
            Vec4i value;
            Vec4i initialValue = this->initialValue.ToVec4i();

            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            const String& z = fieldZ->Text();
            const String& w = fieldW->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y) &&
                String::TryParse(z, value.z) &&
                String::TryParse(w, value.w))
            {
                if (value != initialValue)
                {
                    if (auto history = m_History.Lock())
                    {
                        history->PerformOperation("Edit Vector Field", target, relativePath, initialValue, value);
                    }
                    else
                    {
                        field->SetFieldValue(instance, value);
                        target->OnFieldEdited(field->GetName());
                    }
                }
            }
        }
    }

    void VectorEditorField::OnBeginEditing(NumericEditorField* input)
    {
        if (!IsBound())
            return;

        Ref<Object> target = targets[0].Lock();
        if (target.IsNull())
            return;

        Ptr<FieldType> field;
        void* instance = nullptr;

        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target, field, instance);
        if (!success)
            return;

        TypeId fieldDeclId = field->GetDeclarationTypeId();

        if (fieldDeclId == TYPEID(Vec2))
        {
	        initialValue = field->GetFieldValue<Vec2>(instance);
        }
        else if (fieldDeclId == TYPEID(Vec2i))
        {
            initialValue = field->GetFieldValue<Vec2i>(instance).ToVec2();
        }
        else if (fieldDeclId == TYPEID(Vec3))
        {
            initialValue = field->GetFieldValue<Vec3>(instance);
        }
        else if (fieldDeclId == TYPEID(Vec3i))
        {
            initialValue = field->GetFieldValue<Vec3i>(instance).ToVec3();
        }
        else if (fieldDeclId == TYPEID(Vec4))
        {
            initialValue = field->GetFieldValue<Vec4>(instance);
        }
        else if (fieldDeclId == TYPEID(Vec4i))
        {
            initialValue = field->GetFieldValue<Vec4i>(instance).ToVec4();
        }
    }

    void VectorEditorField::SetFieldValue()
    {
        if (!IsBound())
            return;

        Ref<Object> target = targets[0].Lock();
        if (target.IsNull())
            return;

        Ptr<FieldType> field;
        void* instance = nullptr;

        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target, field, instance);
        if (!success)
            return;

        TypeId fieldDeclId = field->GetDeclarationTypeId();

        if (fieldDeclId == TYPEID(Vec2))
        {
            Vec2 value;
            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y))
            {
                field->SetFieldValue<Vec2>(instance, value);
                target->OnFieldEdited(relativeFieldPath);
            }
        }
        else if (fieldDeclId == TYPEID(Vec2i))
        {
            Vec2i value;
            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y))
            {
                field->SetFieldValue<Vec2i>(instance, value);
                target->OnFieldEdited(relativeFieldPath);
            }
        }
        else if (fieldDeclId == TYPEID(Vec3))
        {
            Vec3 value;
            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            const String& z = fieldZ->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y) &&
                String::TryParse(z, value.z))
            {
                field->SetFieldValue<Vec3>(instance, value);
                target->OnFieldEdited(relativeFieldPath);
            }
        }
        else if (fieldDeclId == TYPEID(Vec3i))
        {
            Vec3i value;
            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            const String& z = fieldZ->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y) &&
                String::TryParse(z, value.z))
            {
                field->SetFieldValue<Vec3i>(instance, value);
                target->OnFieldEdited(relativeFieldPath);
            }
        }
        else if (fieldDeclId == TYPEID(Vec4))
        {
            Vec4 value;
            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            const String& z = fieldZ->Text();
            const String& w = fieldW->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y) &&
                String::TryParse(z, value.z) &&
                String::TryParse(w, value.w))
            {
                field->SetFieldValue<Vec4>(instance, value);
                target->OnFieldEdited(relativeFieldPath);
            }
        }
        else if (fieldDeclId == TYPEID(Vec4i))
        {
            Vec4i value;
            const String& x = fieldX->Text();
            const String& y = fieldY->Text();
            const String& z = fieldZ->Text();
            const String& w = fieldW->Text();
            if (String::TryParse(x, value.x) &&
                String::TryParse(y, value.y) &&
                String::TryParse(z, value.z) &&
                String::TryParse(w, value.w))
            {
                field->SetFieldValue<Vec4i>(instance, value);
                target->OnFieldEdited(relativeFieldPath);
            }
        }
    }

    VectorEditorField::Self& VectorEditorField::VectorType(TypeId type)
    {
        thread_local HashSet floatVectors = { TYPEID(Vec2), TYPEID(Vec3), TYPEID(Vec4) };
        thread_local HashSet intVectors = { TYPEID(Vec2i), TYPEID(Vec3i), TYPEID(Vec4i) };

        vectorTypeId = type;

        if (floatVectors.Exists(type))
        {
            fieldX->NumericType<f32>();
            fieldY->NumericType<f32>();
            fieldZ->NumericType<f32>();
            fieldW->NumericType<f32>();
        }
        else if (intVectors.Exists(type))
        {
            fieldX->NumericType<s32>();
            fieldY->NumericType<s32>();
            fieldZ->NumericType<s32>();
            fieldW->NumericType<s32>();
        }
        else
        {
            return *this;
        }

        fieldX->Enabled(true);
        fieldY->Enabled(true);
        fieldZ->Enabled(false);
        fieldW->Enabled(false);

        if (type == TYPEID(Vec3) || type == TYPEID(Vec3i))
        {
            fieldZ->Enabled(true);
        }
        else if (type == TYPEID(Vec4) || type == TYPEID(Vec4i))
        {
            fieldZ->Enabled(true);
            fieldW->Enabled(true);
        }

        return *this;
    }
}

