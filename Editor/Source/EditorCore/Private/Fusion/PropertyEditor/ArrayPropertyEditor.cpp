#include "EditorCore.h"

namespace CE::Editor
{
    static constexpr const char OperationName[] = "Delete Array Element";

    ArrayPropertyEditor::ArrayPropertyEditor()
    {

    }

    void ArrayPropertyEditor::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();


    }

    void ArrayPropertyEditor::ConstructEditor()
    {
        ConstructDefaultEditor();

        expansionArrow->Visible(true);

        
    }

    bool ArrayPropertyEditor::IsFieldSupported(FieldType* field) const
    {
        if (!field->IsArrayField())
            return false;

        TypeId underlyingTypeId = field->GetUnderlyingTypeId();

        if (underlyingTypeId == TYPEID(Array<>)) // Array of array is not supported
            return false;

        return PropertyEditorRegistry::Get()->IsFieldSupported(underlyingTypeId);
    }

    bool ArrayPropertyEditor::IsFieldSupported(TypeId fieldTypeId) const
    {
        return fieldTypeId == TYPEID(Array<>);
    }

    void ArrayPropertyEditor::ExpandAll(bool expanded, bool recursively)
    {
        Super::ExpandAll(expanded, recursively);

        if (recursively)
        {
            for (PropertyEditor* elementEditor : elementEditors)
            {
                elementEditor->ExpandAll(expanded, recursively);
            }
        }
    }

    void ArrayPropertyEditor::InitTarget(const Array<WeakRef<Object>>& targets, const String& relativeFieldPath)
    {
        right->DestroyAllChildren();

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        auto printError = [&](const String& msg)
            {
                right->AddChild(
                    FNew(FLabel)
                    .FontSize(fontSize)
                    .Text("Error: " + msg)
                    .Foreground(Colors::Red)
                );
            };

        if (targets.GetSize() > 1)
        {
            printError("Multiple objects selected!");
            return;
        }
        if (targets.GetSize() == 0)
        {
            printError("No objects selected!");
            return;
        }

        Ref<Object> target;

        for (const auto& object : targets)
        {
            if (auto lock = object.Lock())
            {
                target = lock;
                break;
            }
        }

        if (target.IsNull())
        {
            printError("No objects selected!");
            return;
        }

        Ptr<FieldType> field = nullptr;
        Ref<Object> outObject = nullptr;
        void* outInstance = nullptr;

        bool foundField = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
            field, outObject, outInstance);

        if (!foundField)
        {
            printError("Cannot find field!");
            return;
        }

        this->arrayEditMode = ArrayEditMode::Default;

        if (field->HasAttribute("ArrayEditorMode") && field->GetAttribute("ArrayEditorMode").IsString())
        {
            String mode = field->GetAttribute("ArrayEditorMode").GetStringValue();
            EnumConstant* constant = GetStaticEnum<ArrayEditMode>()->FindConstantWithName(mode);
            if (constant)
            {
                this->arrayEditMode = (ArrayEditMode)constant->GetValue();
            }
        }

        if (field->HasAttribute("ArrayElementTypeName") && field->GetAttribute("ArrayElementTypeName").IsString())
        {
            hasElementTypeNameOverride = true;
            elementTypeNameOverride = field->GetAttribute("ArrayElementTypeName").GetStringValue();
        }
        else
        {
            hasElementTypeNameOverride = false;
            elementTypeNameOverride = {};
        }

        if (field->HasAttribute("ArrayElementName") && field->GetAttribute("ArrayElementName").IsString())
        {
            elementNameOverride = field->GetAttribute("ArrayElementName").GetStringValue();
        }
        else
        {
            elementNameOverride = {};
        }

        fieldName = field->GetName();
        this->relativeFieldPath = relativeFieldPath;
        this->target = target;
        this->targets = targets;

        FieldNameText(field->GetDisplayName());

        static FBrush deleteIcon = FBrush("/Engine/Resources/Icons/Delete");
        static FBrush addIcon = FBrush("/Engine/Resources/Icons/Add");
        constexpr f32 iconSize = 16;

        Ref<FImageButton> addBtn, deleteBtn;

        (*right)
        .Gap(10)
        (
            FAssignNew(FLabel, countLabel)
            .FontSize(fontSize),

            FNew(FImageButton)
            .Image(addIcon)
            .OnClicked(FUNCTION_BINDING(this, InsertElement))
            .Width(iconSize)
            .Height(iconSize)
            .VAlign(VAlign::Center)
            .Padding(Vec4(1, 1, 1, 1) * 3)
            .Style("Button.Icon")
            .Enabled(this->arrayEditMode == ArrayEditMode::Default),

            FNew(FImageButton)
            .Image(deleteIcon)
            .OnClicked(FUNCTION_BINDING(this, DeleteAllElements))
            .Width(iconSize)
            .Height(iconSize)
            .VAlign(VAlign::Center)
            .Padding(Vec4(1, 1, 1, 1) * 3)
            .Style("Button.Icon")
            .Enabled(this->arrayEditMode == ArrayEditMode::Default)
        );

        UpdateValue();

        isExpanded = PropertyEditorRegistry::Get()->IsExpanded(field);
        UpdateExpansion();
    }

    void ArrayPropertyEditor::UpdateTarget(const Array<WeakRef<Object>>& targets, const String& relativeFieldPath)
    {
        this->targets = targets;
        this->relativeFieldPath = relativeFieldPath;
    }

    void ArrayPropertyEditor::SetSplitRatio(f32 ratio, FSplitBox* excluding)
    {
	    Super::SetSplitRatio(ratio, excluding);

        for (PropertyEditor* elementEditor : elementEditors)
        {
            elementEditor->SetSplitRatio(ratio, excluding);
        }
    }

    bool ArrayPropertyEditor::IsExpandable()
    {
        return true;
    }

    void ArrayPropertyEditor::UpdateValue()
    {
	    Super::UpdateValue();

        Ref<Object> target;

        for (const auto& object : targets)
        {
            if (auto lock = object.Lock())
            {
                target = lock;
                break;
            }
        }

        if (target.IsNull())
        {
            return;
        }

        Ptr<FieldType> field;
        void* instance = nullptr;

        bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target, field, instance);
        if (!success)
        {
	        return;
        }

        int arraySize = field->GetArraySize(instance);

        countLabel->Text(String::Format("{} array elements", arraySize));

        if (arraySize != arrayElements.GetSize())
        {
	        arrayElements = field->GetArrayFieldListPtr(instance);
        }

        bool newElementsCreated = false;

        static FBrush deleteIcon = FBrush("/Engine/Resources/Icons/Delete");
        constexpr f32 iconSize = 16;

	    for (int i = 0; i < arraySize; ++i)
	    {
            const Array<u8>& arrayValue = field->GetFieldValue<Array<u8>>(instance);
            auto arrayInstance = (void*)arrayValue.GetData();
            String arrayElementFieldPath = String::Format("{}[{}]", relativeFieldPath, i);
	        String fieldNameFormat = "Index {}";

            if (i >= elementEditors.GetSize())
            {
	            // Create editor
                PropertyEditor* propertyEditor = PropertyEditorRegistry::Get()->Create(arrayElements[i], objectEditor);

                propertyEditor->SetIndentationLevel(GetIndentationLevel() + 1);

                Ref<Object> elementTarget = target;
                if (TypeInfo* underlyingType = field->GetUnderlyingType())
                {
	                if (underlyingType->IsObject())
	                {
		                if (field->IsStrongRefCounted())
		                {
                            elementTarget = field->GetArrayElementValueAt<Ref<Object>>(i, instance);
		                }
                        else if (field->IsWeakRefCounted())
                        {
                            elementTarget = field->GetArrayElementValueAt<WeakRef<Object>>(i, instance).Lock();
                        }
                        else
                        {
                            elementTarget = field->GetArrayElementValueAt<Object*>(i, instance);
                        }
	                }
                }

                propertyEditor->InitTarget({ target }, arrayElementFieldPath);

                FHorizontalStack& left = *propertyEditor->GetLeft();
            	FHorizontalStack& right = *propertyEditor->GetRight();

                if (arrayElements[i]->IsStructField() && hasElementTypeNameOverride)
                {
                    Ref<FLabel> typeLabel = right.FindChildByName<FLabel>("StructTypeLabel");
                    if (typeLabel)
                    {
                        typeLabel->Text(elementTypeNameOverride);
                    }
                }

                propertyEditor->cachedElementNameFormat = "Index {}";

                if (elementNameOverride.NotEmpty())
                {
                    String leftName = "";

                    for (int j = 0; j < elementNameOverride.GetLength(); ++j)
                    {
                        if (elementNameOverride[j] == '.' && arrayElements[i]->IsStructField())
                        {
                            String subField = "";
                            j++;
                            while (j < elementNameOverride.GetLength())
                            {
                                char c = elementNameOverride[j];
                                if (subField.IsEmpty() && (String::IsAlphabet(c) || c == '_'))
                                {
                                    subField.Append(c);
                                }
                                else if (String::IsAlphabet(c) || String::IsNumeric(c) || c == '_')
                                {
                                    subField.Append(c);
                                }
                                else
                                {
                                    break;
                                }
                                j++;
                            }
                            j--;

                            StructType* structType = (StructType*)arrayElements[i]->GetDeclarationType();
                            void* structInstance = arrayElements[i]->GetFieldInstance(arrayInstance);
                            Ptr<FieldType> structField = structType->FindField(subField);
                            if (structField && (structField->GetDeclarationTypeId() == TYPEID(String) || structField->GetDeclarationTypeId() == TYPEID(CE::Name)))
                            {
                                leftName += structField->GetFieldValueAsString(structInstance);
                            }
                            else
                            {
                                leftName += "." + subField;
                            }
                        }
                        else
                        {
                            leftName.Append(elementNameOverride[j]);
                        }
                    }

                    fieldNameFormat = leftName;
                    propertyEditor->cachedElementNameFormat = fieldNameFormat;
                }

                u32 curIndex = (u32)i;

                if (fixedInputWidth > 0)
                {
                    propertyEditor->FixedInputWidth(fixedInputWidth);
                }

            	right
                (
                    FNew(FImageButton)
                    .Image(deleteIcon)
                    .OnClicked([this, curIndex]
                    {
                        DeleteElement(curIndex);
                    })
                    .Width(iconSize)
                    .Height(iconSize)
                    .VAlign(VAlign::Center)
                    .Padding(Vec4(1, 1, 1, 1) * 3)
                    .Margin(Vec4(5, 0, 5, 0))
                    .Style("Button.Icon")
                    .Enabled(this->arrayEditMode == ArrayEditMode::Default)
                );

                elementEditors.Add(propertyEditor);

                expansionStack->AddChild(propertyEditor);

                newElementsCreated = true;
            }

            // Use editor
	    	elementEditors[i]->UpdateTarget({ target }, arrayElementFieldPath);

	        elementEditors[i]->FieldNameText(String::Format(elementEditors[i]->cachedElementNameFormat, i));

            elementEditors[i]->UpdateValue();
	    }

	    for (int i = (int)elementEditors.GetSize() - 1; i >= arraySize; i--)
	    {
            Ref<PropertyEditor> editor = elementEditors[i];
            expansionStack->RemoveChild(editor.Get());
            editor->QueueDestroy();
            elementEditors.RemoveAt(i);
	    }

        if (newElementsCreated && objectEditor)
        {
            objectEditor->ApplySplitRatio(nullptr);
        }
    }

    PropertyEditor& ArrayPropertyEditor::FixedInputWidth(f32 width)
    {
	    Super::FixedInputWidth(width);

        fixedInputWidth = width;

        for (PropertyEditor* elementEditor : elementEditors)
        {
            elementEditor->FixedInputWidth(width);
        }

        return *this;
    }

    void ArrayPropertyEditor::InsertElement()
    {
        Ref<EditorHistory> history = objectEditor->GetEditorHistory();
        Ref<Object> target = this->target.Lock();
        WeakRef<Object> targetWeak = target;
        CE::Name arrayFieldPath = relativeFieldPath;

        if (history.IsValid())
        {
            history->PerformOperation("Insert Array Element", target,
                [arrayFieldPath, targetWeak](const Ref<EditorOperation>& operation)
                {
                    if (auto target = targetWeak.Lock())
                    {
                        Ptr<FieldType> arrayField;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(arrayFieldPath, target,
                            arrayField, instance);
                        if (!success || !arrayField->IsArrayField())
                        {
                            return false;
                        }

                        arrayField->InsertArrayElement(instance);
                        operation->SetArrayIndex(arrayField->GetArraySize(instance) - 1);

                        target->OnFieldChanged(arrayFieldPath);

                        return true;
                    }

                    return false;
                },
                [arrayFieldPath, targetWeak](const Ref<EditorOperation>& operation)
                {
                    if (auto target = targetWeak.Lock())
                    {
                        Ptr<FieldType> arrayField;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(arrayFieldPath, target,
                            arrayField, instance);
                        if (!success || !arrayField->IsArrayField())
                        {
                            return false;
                        }

                        int index = operation->GetArrayIndex();

                        if (index < 0 || index >= (int)arrayField->GetArraySize(instance))
                        {
	                        return false;
                        }

                        arrayField->DeleteArrayElement(instance, index);

                        target->OnFieldChanged(arrayFieldPath);

                        return true;
                    }

                    return false;
                });
        }
        else
        {
            Ptr<FieldType> field;
            void* instance = nullptr;
            bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                field, instance);
            if (!success)
            {
                return;
            }

            field->InsertArrayElement(instance);
        }

        UpdateValue();
    }

    void ArrayPropertyEditor::DeleteAllElements()
    {
        Ref<EditorHistory> history = objectEditor->GetEditorHistory();
        Ref<Object> target = this->target.Lock();
        WeakRef<Object> targetWeak = target;
        CE::Name arrayFieldPath = relativeFieldPath;

        if (history.IsValid())
        {
            Ptr<FieldType> field;
            void* instance = nullptr;
            bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                field, instance);
            if (!success)
            {
                return;
            }

            TypeInfo* underlyingType = field->GetUnderlyingType();
            if (underlyingType == nullptr)
            {
	            return;
            }
            
            MemoryStream jsonData = MemoryStream(1024, true);
            jsonData.SetAsciiMode(true);
            jsonData.SetAutoResizeIncrement(1024);

            JsonFieldSerializer serializer{ { field }, instance };
            serializer.Serialize(&jsonData);
            jsonData.Seek(0);

            history->PerformOperation(OperationName, target,
                [targetWeak, arrayFieldPath]
                (const Ref<EditorOperation>& operation)
                {
                    if (auto target = targetWeak.Lock())
                    {
                        Ptr<FieldType> field;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(arrayFieldPath, target,
                            field, instance);
                        if (!success)
                            return false;

                        while (field->GetArraySize(instance) > 0)
                        {
                            field->DeleteArrayElement(instance, 0);
                        }

                        target->OnFieldChanged(arrayFieldPath);

                        return true;
                    }

                    return false;
                },
                [targetWeak, arrayFieldPath, jsonData]
                (const Ref<EditorOperation>& operation)
                {
                    if (auto target = targetWeak.Lock())
                    {
                        Ptr<FieldType> field;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(arrayFieldPath, target,
                            field, instance);
                        if (!success)
                            return false;

                        MemoryStream& json = const_cast<MemoryStream&>(jsonData);
                        json.Seek(0);

                        JsonFieldDeserializer deserializer{ { field }, instance };
                        deserializer.Deserialize(&json);

                        target->OnFieldChanged(arrayFieldPath);

                        return true;
                    }

                    return false;
                });
        }
        else
        {
            Ptr<FieldType> field;
            void* instance = nullptr;
            bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                field, instance);
            if (!success)
            {
                return;
            }

            while (field->GetArraySize(instance) > 0)
            {
                field->DeleteArrayElement(instance, 0);
            }
        }

        UpdateValue();
    }

    void ArrayPropertyEditor::DeleteElement(u32 index)
    {
        Ref<EditorHistory> history = objectEditor->GetEditorHistory();
        Ref<Object> target = this->target.Lock();
        WeakRef<Object> targetWeak = target;
        CE::Name arrayFieldPath = relativeFieldPath;

        if (history.IsValid())
        {
            Ptr<FieldType> field;
            void* instance = nullptr;
            bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                field, instance);
            if (!success || !field->IsArrayField())
                return;

            Ptr<FieldType> elementField = field->GetArrayFieldElementPtr(instance, index);

            if (elementField == nullptr)
                return;

            TypeInfo* declType = elementField->GetDeclarationType();
            if (declType == nullptr)
                return;

            TypeId declId = declType->GetTypeId();

            void* arrayInstance = field->GetArrayInstance(instance);

            MemoryStream jsonData = MemoryStream(1024, true);
            jsonData.SetAsciiMode(true);
            jsonData.SetAutoResizeIncrement(1024);

            JsonFieldSerializer serializer{ { field }, instance };
            serializer.Serialize(&jsonData);
            jsonData.Seek(0);

            history->PerformOperation(OperationName, target,
                [targetWeak, index, arrayFieldPath]
                (const Ref<EditorOperation>& operation)
                {
                    if (auto target = targetWeak.Lock())
                    {
                        Ptr<FieldType> field;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(arrayFieldPath, target,
                            field, instance);
                        if (!success)
                            return false;
                        if (index >= field->GetArraySize(instance))
                            return false;

                        field->DeleteArrayElement(instance, index);

                        target->OnFieldChanged(arrayFieldPath);

                        return true;
                    }

                    return false;
                },
                [targetWeak, index, arrayFieldPath, jsonData]
                (const Ref<EditorOperation>& operation)
                {
                    if (auto target = targetWeak.Lock())
                    {
                        Ptr<FieldType> field;
                        void* instance = nullptr;
                        bool success = target->GetClass()->FindFieldInstanceRelative(arrayFieldPath, target,
                            field, instance);
                        if (!success)
                            return false;

                        MemoryStream& json = const_cast<MemoryStream&>(jsonData);
                        json.Seek(0);

                        JsonFieldDeserializer deserializer{ { field }, instance };
                        deserializer.Deserialize(&json);

                        target->OnFieldChanged(arrayFieldPath);

                        return true;
                    }

                    return false;
                });
        }
        else
        {
            Ptr<FieldType> field;
            void* instance = nullptr;
            bool success = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
                field, instance);
            if (!success)
            {
                return;
            }

            if (index < field->GetArraySize(instance))
            {
                field->DeleteArrayElement(instance, index);
            }
        }

        UpdateValue();
    }

}

