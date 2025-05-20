#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorHistory : public Object
    {
        CE_CLASS(EditorHistory, Object)
    protected:

        EditorHistory();

    public: // - Public API -

        void PerformOperation(const String& name, const Ref<Object>& target, 
            const EditorOperationDelegate& execute, 
            const EditorOperationDelegate& unexecute,
            const SubClass<EditorOperation>& operationClass = EditorOperation::StaticClass());

        void PerformOperation(const String& name, const Array<Ref<Object>>& targets, 
            const EditorOperationDelegate& execute, 
            const EditorOperationDelegate& unexecute,
            const SubClass<EditorOperation>& operationClass = EditorOperation::StaticClass());

        template<typename T>
        void PerformOperation(const String& name, Ref<Object> target,
            const CE::Name& relativeFieldPath,
            const T& initialValue, const T& newValue)
        {
            WeakRef<Object> targetRef = target;

            PerformOperation(name, target,
                [targetRef, relativeFieldPath, newValue](const Ref<EditorOperation>& operation)
                {
                    if (auto target = targetRef.Lock())
                    {
                        Ptr<FieldType> field;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                            field, instance);
                        if (!success)
                        {
                            return false;
                        }

                        field->SetFieldValue<T>(instance, newValue);
                        if (!operation->IsEditorGui())
                        {
	                        target->OnFieldChanged(relativeFieldPath);
                        }
                        else
                        {
                            target->OnFieldEdited(relativeFieldPath);
                        }

                        return true;
                    }

                    return false;
                },
                [targetRef, relativeFieldPath, initialValue](const Ref<EditorOperation>& operation)
                {
                    if (auto target = targetRef.Lock())
                    {
                        Ptr<FieldType> field;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                            field, instance);
                        if (!success)
                        {
                            return false;
                        }

                        field->SetFieldValue<T>(instance, initialValue);

                        if (!operation->IsEditorGui())
                        {
                            target->OnFieldChanged(relativeFieldPath);
                        }
                        else
                        {
                            target->OnFieldEdited(relativeFieldPath);
                        }

                        return true;
                    }

                    return false;
                });
        }

        void PerformObjectOperation(const String& name, const Ref<Object>& target,
            const CE::Name& relativeFieldPath,
            WeakRef<Object> initialValue, WeakRef<Object> newValue)
        {
            WeakRef<Object> targetRef = target;

            PerformOperation(name, target,
                [targetRef, relativeFieldPath, newValue](const Ref<EditorOperation>& operation)
                {
                    if (Ref<Object> target = targetRef.Lock())
                    {
                        Ptr<FieldType> field;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                            field, instance);
                        if (!success || !field->IsObjectField())
                        {
                            return false;
                        }

                        if (field->IsStrongRefCounted())
                        {
                            field->SetFieldValue<Ref<Object>>(instance, newValue.Lock());
                        }
                        else if (field->IsWeakRefCounted())
                        {
                            field->SetFieldValue<WeakRef<Object>>(instance, newValue);
                        }
                        else
                        {
                            field->SetFieldValue<Object*>(instance, newValue.Get());
                        }

                        if (!operation->IsEditorGui())
                        {
                            target->OnFieldChanged(relativeFieldPath);
                        }
                        else
                        {
                            target->OnFieldEdited(relativeFieldPath);
                        }

                        return true;
                    }

                    return false;
                },
                [targetRef, relativeFieldPath, initialValue](const Ref<EditorOperation>& operation)
                {
                    if (Ref<Object> target = targetRef.Lock())
                    {
                        Ptr<FieldType> field;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                            field, instance);
                        if (!success || !field->IsObjectField())
                        {
                            return false;
                        }

                        if (field->IsStrongRefCounted())
                        {
                            field->SetFieldValue<Ref<Object>>(instance, initialValue.Lock());
                        }
                        else if (field->IsWeakRefCounted())
                        {
                            field->SetFieldValue<WeakRef<Object>>(instance, initialValue);
                        }
                        else
                        {
                            field->SetFieldValue<Object*>(instance, initialValue.Get());
                        }

                        if (!operation->IsEditorGui())
                        {
                            target->OnFieldChanged(relativeFieldPath);
                        }
                        else
                        {
                            target->OnFieldEdited(relativeFieldPath);
                        }

                        return true;
                    }

                    return false;
                });
        }

        void Undo();
        void Redo();

        ScriptEvent<void(EditorHistory*)> onTargetModified;

    protected: // - Internal -

        static constexpr u32 OperationStackSize = 512;
        using OperationStack = Array<Ref<EditorOperation>>;
        
        OperationStack historyStack;
        int topIndex = -1;

    };
    
}

#include "EditorHistory.rtti.h"
