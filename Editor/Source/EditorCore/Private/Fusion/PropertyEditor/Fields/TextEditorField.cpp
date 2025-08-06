#include "EditorCore.h"

namespace CE::Editor
{

    TextEditorField::TextEditorField()
    {

    }

    void TextEditorField::Construct()
    {
        Super::Construct();

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        Child(
            FAssignNew(FTextInput, input)
            .FontSize(fontSize)
            .OnTextEditingFinished(FUNCTION_BINDING(this, OnFinishEdit))
            .OnTextEdited(FUNCTION_BINDING(this, OnTextFieldEdited))
            .OnBeginEditing(FUNCTION_BINDING(this, OnBeginEditing))
            .Width(100)
            .VAlign(VAlign::Center)
        );
    }

    EditorField& TextEditorField::FixedInputWidth(f32 width)
    {
        input->Width(width);
        return *this;
    }

    bool TextEditorField::CanBind(FieldType* field)
    {
        thread_local HashSet types = { TYPEID(String), TYPEID(CE::Name) };

        return types.Exists(field->GetDeclarationTypeId());
    }

    bool TextEditorField::CanBind(TypeId boundTypeId, TypeId underlyingTypeId)
    {
        thread_local HashSet types = { TYPEID(String), TYPEID(CE::Name) };

        return types.Exists(boundTypeId);
    }

    void TextEditorField::UpdateValue()
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

        if (!success)
            return;

        const String& string = field->GetFieldValueAsString(instance);

        input->Text(string);
    }

    void TextEditorField::OnTextFieldEdited(FTextInput*)
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

        if (!success)
            return;

        TypeId fieldDeclId = field->GetDeclarationTypeId();

        const String& string = input->Text();

        if (fieldDeclId == TYPEID(String))
        {
            field->SetFieldValue<String>(instance, string);
            target->OnFieldEdited(relativeFieldPath);
        }
        else if (fieldDeclId == TYPEID(CE::Name))
        {
            field->SetFieldValue<CE::Name>(instance, string);
            target->OnFieldEdited(relativeFieldPath);
        }
    }

    void TextEditorField::OnBeginEditing(FTextInput*)
    {
        initialValue = input->Text();
    }

    void TextEditorField::OnFinishEdit(FTextInput*)
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

        if (!success)
            return;

        TypeId fieldDeclId = field->GetDeclarationTypeId();

        const String& string = input->Text();

        if (fieldDeclId == TYPEID(String))
        {
            if (auto history = m_History.Lock())
            {
                history->PerformOperation<String>("Edit Text Field", target, relativeFieldPath,
                    initialValue, string);
            }
            else
            {
                SetValueDirect<String>(string);
            }
        }
        else if (fieldDeclId == TYPEID(CE::Name))
        {
            if (auto history = m_History.Lock())
            {
                history->PerformOperation<CE::Name>("Edit Text Field", target, relativeFieldPath,
                                    initialValue, string);
            }
            else
            {
                SetValueDirect<CE::Name>(string);
            }
        }
    }

}

