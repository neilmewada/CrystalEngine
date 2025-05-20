#pragma once

namespace CE::Editor
{

    CLASS(Abstract)
    class EDITORCORE_API EditorField : public FCompoundWidget
    {
        CE_CLASS(EditorField, FCompoundWidget)
    protected:

        EditorField();

        void Construct() override;

        void OnBeginDestroy() override;

    public: // - Public API -

        virtual bool CanBind(FieldType* field);
        virtual bool CanBind(TypeId boundTypeId, TypeId underlyingTypeId) = 0;

        bool CanBind(const Ref<Object>& target, const CE::Name& relativeFieldPath);

        virtual Self& BindField(const Ref<Object>& target, const CE::Name& relativeFieldPath);

        virtual Self& UnbindField();

        bool IsBound() const { return isBound; }

        virtual EditorField& FixedInputWidth(f32 width);

        virtual void OnBind() {}

        virtual void UpdateValue() = 0;

    protected: // - Internal -

        template<typename T>
        bool SetValueDirect(const T& value)
        {
            if (auto target = targets[0].Lock())
            {
                Ptr<FieldType> field;
                void* instance = nullptr;
                bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                    field, instance);
                if (success)
                {
                    field->SetFieldValue<T>(instance, value);
                    target->OnFieldEdited(field->GetName());
                    return true;
                }
            }

            return false;
        }

        template<typename T>
        const T& GetValueDirect() const
        {
            static T empty{};

            if (auto target = targets[0].Lock())
            {
                Ptr<FieldType> field;
                void* instance = nullptr;
                bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                    field, instance);
                if (success)
                {
                    return field->GetFieldValue<T>(instance);
                }
            }

            return empty;
        }

        bool isBound = false;
        Array<WeakRef<Object>> targets;
        CE::Name relativeFieldPath;

        TypeId fieldDeclId;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(WeakRef<EditorHistory>, History);

        FUSION_WIDGET;
        friend class PropertyEditor;
        friend class ArrayPropertyEditor;
    };
    
}

#include "EditorField.rtti.h"
