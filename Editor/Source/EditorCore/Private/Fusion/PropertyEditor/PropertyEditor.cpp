#include "EditorCore.h"

namespace CE::Editor
{

    PropertyEditor::PropertyEditor()
    {
        m_HoverRowBackground = Color::RGBA(47, 47, 47);
    }

    void PropertyEditor::Construct()
    {
        Super::Construct();

        ConstructEditor();

        if (splitBox)
        {
            (*splitBox)
                .SplitterHoverBackground(Color::Clear())
                .SplitterBackground(Color::RGBA(26, 26, 26))
                .SplitterSize(4.0f)
                .SplitterDrawRatio(0.25f)
        	;
        }
    }

    void PropertyEditor::ConstructDefaultEditor()
    {
        FBrush caretDown = FBrush("/Engine/Resources/Icons/CaretDown");

        Child(
            FAssignNew(FVerticalStack, contentStack)
            .Padding(Vec4())
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
                FNew(DetailsRow)
                .OnDoubleClicked(FUNCTION_BINDING(this, ToggleExpansion))
                .HAlign(HAlign::Fill)
                .Padding(Vec4())
                .Style("Button.DetailsRow")
                .Name("DetailsRow")
                (
                    FAssignNew(FSplitBox, splitBox)
                    .OnSplitterDragged([this](FSplitBox*)
                    {
                        if (objectEditor)
                        {
                            objectEditor->SetSplitRatio(left->FillRatio(), splitBox);
                        }
                    })
                    .Direction(FSplitDirection::Horizontal)
                    .SplitterHoverBackground(Color::Clear())
                    .SplitterBackground(Color::RGBA(26, 26, 26))
                    .SplitterSize(4.0f)
                    .SplitterDrawRatio(0.25f)
                    .HAlign(HAlign::Fill)
                    (
                        FAssignNew(FHorizontalStack, left)
                        .ContentHAlign(HAlign::Left)
                        .ContentVAlign(VAlign::Center)
                        .ClipChildren(true)
                        .FillRatio(0.35f)
                        .Padding(Vec4(2, 1.0f, 2, 1.0f) * 5)
                        .MinHeight(20)
                        (
                            FAssignNew(FImageButton, expansionArrow)
                            .Image(caretDown)
                            .ImageWidth(11)
                            .ImageHeight(11)
                            .OnClicked(FUNCTION_BINDING(this, ToggleExpansion))
                            .VAlign(VAlign::Center)
                            .Margin(Vec4(0, 0, 5, 0))
                            .Style("ExpandCaretButton")
                            .Visible(IsExpandable()),

                            FAssignNew(FLabel, fieldNameLabel)
                            .Text("Field Name")
                            .FontSize(10)
                        ),

                        FAssignNew(FHorizontalStack, right)
                        .ContentHAlign(HAlign::Left)
                        .ContentVAlign(VAlign::Center)
                        .ClipChildren(true)
                        .FillRatio(0.65f)
                        .Padding(Vec4(2, 1, 2, 1) * 5)
                        .MinHeight(20)
                    )
                ),

                FAssignNew(FVerticalStack, expansionStack)
                .HAlign(HAlign::Fill)
                .Padding(Vec4())
                .Enabled(IsExpandable() && isExpanded)
            )
        );

        UpdateExpansion();
    }

    void PropertyEditor::ConstructEditor()
    {
        ConstructDefaultEditor();
    }

    bool PropertyEditor::IsFieldSupported(FieldType* field) const
    {
        return IsFieldSupported(field->GetDeclarationTypeId());
    }

    bool PropertyEditor::IsFieldSupported(TypeId fieldTypeId) const
    {
        if (fieldTypeId == TYPEID(Array<>))
            return false;

        thread_local HashSet supportedFields = {
            TYPEID(bool),
            TYPEID(s8), TYPEID(u8),
            TYPEID(s16), TYPEID(u16),
            TYPEID(s32), TYPEID(u32),
            TYPEID(s64), TYPEID(u64),
            TYPEID(f32), TYPEID(f64),
            TYPEID(String), TYPEID(CE::Name),
            TYPEID(Vec2), TYPEID(Vec2i),
            TYPEID(Vec3), TYPEID(Vec3i),
            TYPEID(Vec4), TYPEID(Vec4i),
            TYPEID(Color)
        };

        if (supportedFields.Exists(fieldTypeId))
            return true;

        TypeInfo* type = GetTypeInfo(fieldTypeId);
        if (type == nullptr)
            return false;

        return type->IsEnum() || type->IsStruct() || type->IsClass();
    }

    bool PropertyEditor::IsExpandable()
    {
        return fieldDeclType != nullptr && fieldDeclType->IsStruct();
    }

    void PropertyEditor::UpdateValue()
    {
        if (editorField)
        {
            editorField->UpdateValue();
        }

        for (const auto& property : structProperties)
        {
            property->UpdateValue();
        }
    }

    void PropertyEditor::ParseShowIfCondition(Ptr<FieldType> field, Ref<Object> target, const String& parentFieldRelativePath)
    {
        if (!target || !field || parentFieldRelativePath.IsEmpty())
            return;

        showCondition.valid = false;
        showCondition.isEquality = false;
        showCondition.comparisonFieldName = "";
        showCondition.showIfValue = "";

        if (field->HasAttribute("ShowIf") && field->GetAttribute("ShowIf").IsString())
        {
            String showIfConditionStr = field->GetAttribute("ShowIf").GetStringValue();

            for (int j = 0; j < showIfConditionStr.GetLength(); ++j)
            {
                if (showIfConditionStr[j] == '.')
                {
                    j++;
                    while (j < showIfConditionStr.GetLength())
                    {
                        char c = showIfConditionStr[j];
                        if (showCondition.comparisonFieldName.IsEmpty() && (String::IsAlphabet(c) || c == '_'))
                        {
                            showCondition.comparisonFieldName.Append(c);
                        }
                        else if (String::IsAlphabet(c) || String::IsNumeric(c) || c == '_')
                        {
                            showCondition.comparisonFieldName.Append(c);
                        }
                        else
                        {
                            break;
                        }
                        j++;
                    }
                    j--;
                }
                else if (showCondition.comparisonFieldName.NotEmpty() &&
                    showIfConditionStr.GetSubstring(j).StartsWith("=="))
                {
                    showCondition.isEquality = true;
                    j += 2;

                    // Skip whitespaces
                    while (showIfConditionStr[j] == ' ' && j < showIfConditionStr.GetLength())
                    {
                        j++;
                    }

                    while (j < showIfConditionStr.GetLength())
                    {
                        char c = showIfConditionStr[j];
                        showCondition.showIfValue.Append(c);
                        j++;
                    }
                }
            }

            Ptr<FieldType> foundField = nullptr;
            Ref<Object> outObject = nullptr;
            void* outInstance = nullptr;

            bool found = target->GetClass()->FindFieldInstanceRelative(parentFieldRelativePath + "." + showCondition.comparisonFieldName,
                target, foundField, outObject, outInstance);

            if (found)
            {
                showCondition.valid = true;
            }
        }

        ValidateShowIfCondition(target, parentFieldRelativePath);
    }

    void PropertyEditor::ValidateShowIfCondition(Ref<Object> target, const String& parentFieldRelativePath)
    {
        if (!showCondition.valid)
            return;

        Ptr<FieldType> foundField = nullptr;
        Ref<Object> outObject = nullptr;
        void* outInstance = nullptr;

        bool found = target->GetClass()->FindFieldInstanceRelative(parentFieldRelativePath + "." + showCondition.comparisonFieldName,
            target, foundField, outObject, outInstance);

        if (!found)
            return;

        if (showCondition.isEquality)
        {
            Enabled(foundField->GetFieldValueAsString(outInstance) == showCondition.showIfValue);
        }
        else if (foundField->GetDeclarationTypeId() == TYPEID(bool))
        {
            Enabled(foundField->GetFieldValue<bool>(outInstance));
        }
    }

    void PropertyEditor::InitTarget(const Array<WeakRef<Object>>& targets, const String& relativeFieldPath)
    {
        right->DestroyAllChildren();

        auto printError = [&](const String& msg)
        {
            right->AddChild(
                FNew(FLabel)
                .FontSize(10)
                .Text("Error: " + msg)
                .Foreground(Color::Red())
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

        if (field->HasAttribute("StructTypeName") && field->GetAttribute("StructTypeName").IsString())
        {
            hasStructTypeNameOverride = true;
            structTypeNameOverride = field->GetAttribute("StructTypeName").GetStringValue();
        }
        else
        {
            hasStructTypeNameOverride = false;
            structTypeNameOverride = {};
        }

        showCondition.valid = false;
        showCondition.isEquality = false;
        showCondition.comparisonFieldName = "";
        showCondition.showIfValue = "";

        for (int j = relativeFieldPath.GetLength() - 1; j >= 0; --j)
        {
            char c = relativeFieldPath[j];
            if (c != '_' && !String::IsAlphabet(c) && !String::IsNumeric(c))
            {
                if (c == '.' && j != relativeFieldPath.GetLength() - 1)
                {
                    ParseShowIfCondition(field, target, relativeFieldPath.GetSubstring(0, j));
                }
                break;
            }
        }

        fieldName = field->GetName();
        this->relativeFieldPath = relativeFieldPath;

        FieldNameText(field->GetDisplayName());

        fieldDeclType = field->GetDeclarationType();
        if (fieldDeclType == nullptr)
        {
            printError("Cannot find field type!");
	        return;
        }

        this->target = targets[0];
        this->targets = targets;
        structProperties.Clear();

        isExpanded = PropertyEditorRegistry::Get()->IsExpanded(field);
        UpdateExpansion();

        TypeId fieldDeclId = fieldDeclType->GetTypeId();

        if (fieldDeclType->IsStruct()) // - Struct Editor -
        {
            StructType* structType = (StructType*)fieldDeclType;
            String structTypeName = structType->GetName().GetString();
            if (hasStructTypeNameOverride)
            {
                structTypeName = structTypeNameOverride.GetString();
            }

            right->AddChild(
                FNew(FLabel)
                .FontSize(10)
                .Text(structTypeName)
                .Name("StructTypeLabel")
            );

            for (int i = 0; i < structType->GetFieldCount(); ++i)
            {
                Ptr<FieldType> structField = structType->GetFieldAt(i);
                if (!structField->IsEditAnywhere())
                {
                    continue;
                }

                // Create editor
                PropertyEditor* propertyEditor = PropertyEditorRegistry::Get()->Create(structField, objectEditor);
                if (propertyEditor == nullptr)
                {
                    continue;
                }

                structProperties.Add(propertyEditor);

                propertyEditor->parentObjectEditor = parentObjectEditor;
                propertyEditor->parentEditor = this;

                propertyEditor->SetIndentationLevel(GetIndentationLevel() + 1);
                
                propertyEditor->InitTarget(this->targets,
                    relativeFieldPath + "." + structField->GetName().GetString());

                expansionStack->AddChild(propertyEditor);
            }

            expansionArrow->Visible(true);
        }
        else if (fieldDeclType->IsVectorType())
        {
            right->AddChild(
				FNew(VectorEditorField)
                .VectorType(fieldDeclType->GetTypeId())
                .Assign(editorField)
                .BindField(target, relativeFieldPath)
                .History(objectEditor->GetEditorHistory())
                .FillRatio(1.0f)
            );
        }
        else if (field->IsNumericField())
        {
            right->AddChild(
                FNew(NumericEditorField)
                .NumericType(fieldDeclType->GetTypeId())
                .ColorTagVisible(false)
                .Assign(editorField)
                .BindField(target, relativeFieldPath)
                .History(objectEditor->GetEditorHistory())
                .Width(100)
            );
        }
        else if (fieldDeclType->IsEnum())
        {
            right->AddChild(
				FNew(EnumEditorField)
                .Assign(editorField)
                .BindField(target, relativeFieldPath)
                .History(objectEditor->GetEditorHistory())
                .FillRatio(1.0f)
            );
        }
        else if (field->GetDeclarationTypeId() == TYPEID(bool))
        {
            right->AddChild(
				FNew(BoolEditorField)
                .Assign(editorField)
                .BindField(target, relativeFieldPath)
                .History(objectEditor->GetEditorHistory())
                .HAlign(HAlign::Left)
                .VAlign(VAlign::Center)
            );
        }
        else if (fieldDeclId == TYPEID(Color))
        {
            right->AddChild(
                FNew(ColorEditorField)
                .Assign(editorField)
                .BindField(target, relativeFieldPath)
                .History(objectEditor->GetEditorHistory())
                .HAlign(HAlign::Left)
                .VAlign(VAlign::Center)
            );
        }
        else if (fieldDeclId == TYPEID(String) || fieldDeclId == TYPEID(CE::Name))
        {
            right->AddChild(
                FNew(TextEditorField)
                .Assign(editorField)
                .BindField(target, relativeFieldPath)
                .History(objectEditor->GetEditorHistory())
                .HAlign(HAlign::Left)
                .VAlign(VAlign::Center)
            );
        }
        else if (field->IsObjectField())
        {
            // TODO: Object reference editor, check if it is asset or scene-object reference

            right->AddChild(
                FNew(ObjectEditorField)
                .ObjectEditorOwner(objectEditor)
                .Assign(editorField)
                .BindField(target, relativeFieldPath)
                .History(objectEditor->GetEditorHistory())
                .HAlign(HAlign::Left)
                .VAlign(VAlign::Center)
            );
        }
    }

    void PropertyEditor::UpdateTarget(const Array<WeakRef<Object>>& targets, const String& relativeFieldPath)
    {
        if (!editorField)
            return;

        Ref<Object> target;

        for (const auto& object : targets)
        {
            if (auto lock = object.Lock())
            {
                target = lock;
                break;
            }
        }

        Ptr<FieldType> field = nullptr;
        Ref<Object> outObject = nullptr;
        void* outInstance = nullptr;

        bool foundField = target->GetClass()->FindFieldInstanceRelative(relativeFieldPath, target,
            field, outObject, outInstance);

        if (foundField)
        {
            editorField->targets = targets;
            editorField->relativeFieldPath = relativeFieldPath;
        }
    }

    void PropertyEditor::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        painter->SetPen(FPen(Color::RGBA(26, 26, 26), 1));
        painter->SetBrush(FBrush());

        f32 expansionHeight = 0;
        if (expansionStack->Enabled())
        {
            expansionHeight = expansionStack->GetComputedSize().height;
        }

        constexpr f32 height = 1.0f;
        Vec2 pos = Vec2(0, computedSize.height - height - expansionHeight + 1);
        Vec2 size = Vec2(computedSize.width, height);

        painter->DrawLine(pos, pos + Vec2(size.x, 0));
    }

    void PropertyEditor::SetIndentationLevel(int value)
    {
        indentation = value;

        left->Translation(Vec2(indentation * 10, 0));
    }

    f32 PropertyEditor::GetSplitRatio() const
    {
        return left->FillRatio();
    }

    void PropertyEditor::SetSplitRatio(f32 ratio)
    {
        ratio = Math::Clamp(ratio, 0.01f, 0.99f);
        left->FillRatio(ratio);
        right->FillRatio(1.0f - ratio);
    }

    void PropertyEditor::SetSplitRatio(f32 ratio, FSplitBox* excluding)
    {
        if (splitBox != excluding)
        {
            SetSplitRatio(ratio);
        }

        for (const auto& structProp : structProperties)
        {
            structProp->SetSplitRatio(ratio, excluding);
        }
    }

    PropertyEditor& PropertyEditor::FixedInputWidth(f32 width)
    {
        if (editorField != nullptr)
        {
	        editorField->FixedInputWidth(width);
        }

        return *this;
    }

    void PropertyEditor::ExpandAll(bool expanded, bool recursively)
    {
        if (!IsExpandable() || (!recursively && isExpanded == expanded))
            return;

        isExpanded = expanded;

        UpdateExpansion();

        if (isExpanded)
        {
            OnExpand();
        }
        else
        {
            OnCollapse();
        }

        if (recursively)
        {
            for (Ref<PropertyEditor> structProperty : structProperties)
            {
                structProperty->ExpandAll(expanded, recursively);
            }
        }
    }

    void PropertyEditor::ToggleExpansion()
    {
        if (!IsExpandable())
            return;

        isExpanded = !isExpanded;

        UpdateExpansion();

        if (isExpanded)
        {
            OnExpand();
        }
        else
        {
            OnCollapse();
        }
    }

    void PropertyEditor::UpdateExpansion()
    {
        if (!IsExpandable())
            return;

        expansionArrow->GetImage()->Angle(isExpanded ? 0 : -90);
        expansionStack->Enabled(isExpanded);
    }

    
}

