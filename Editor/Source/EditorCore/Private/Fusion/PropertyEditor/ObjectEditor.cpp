#include "EditorCore.h"

namespace CE::Editor
{

    ObjectEditor::ObjectEditor()
    {

    }

    ObjectEditor::~ObjectEditor()
    {
        
    }

    void ObjectEditor::SetEditorHistory(const Ref<EditorHistory>& history)
    {
        this->history = history;
    }

    void ObjectEditor::Construct()
    {
	    Super::Construct();

        Child(
            FAssignNew(FVerticalStack, content)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
        );
    }

    void ObjectEditor::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        for (const auto& targetObjectUuid : targetObjectUuids)
        {
            ObjectListener::RemoveListener(targetObjectUuid, this);
        }
    }

    void ObjectEditor::OnObjectFieldChanged(Uuid objectUuid, const CE::Name& fieldPath)
    {
        if (!targetObjectUuids.Exists(objectUuid))
            return;

        // TODO: Fix this for nested struct fields

        String fieldNameStr = fieldPath.GetString();
        if (fieldNameStr.Contains('.') || fieldNameStr.Contains('['))
        {
            Array<String> splits;
            fieldNameStr.Split({"[", "."}, splits);
            fieldNameStr = splits[0];
        }

	    for (PropertyEditor* propertyEditor : propertyEditors)
	    {
            if (propertyEditor->IsOfType<ArrayPropertyEditor>())
            {
                if (fieldPath.GetString().Contains(propertyEditor->GetRelativeFieldPath().GetString()))
                {
                    propertyEditor->UpdateValue();
                }
                continue;
            }

            if (propertyEditor->relativeFieldPath.GetString().Contains(fieldNameStr))
            {
	            propertyEditor->UpdateValue();
            }
	    }
    }

    void ObjectEditor::OnObjectFieldEdited(Uuid objectUuid, const CE::Name& fieldPath)
    {
        if (!targetObjectUuids.Exists(objectUuid))
            return;

        // TODO: Fix this for nested struct fields

        String fieldNameStr = fieldPath.GetString();
        if (fieldNameStr.Contains('.') || fieldNameStr.Contains('['))
        {
            Array<String> splits;
            fieldNameStr.Split({ "[", "." }, splits);
            fieldNameStr = splits[0];
        }

        for (PropertyEditor* propertyEditor : propertyEditors)
        {
            if (propertyEditor->IsOfType<ArrayPropertyEditor>())
            {
                if (fieldPath.GetString().Contains(propertyEditor->GetRelativeFieldPath().GetString()))
                {
                    propertyEditor->UpdateValue();
                }
                continue;
            }

			EditorField* editorField = propertyEditor->GetEditorField();
            if (editorField == nullptr)
				continue;
			if (!editorField->IsBound())
				continue;

            if (editorField->IsOfType<ObjectEditorField>())
            {
                if (propertyEditor->relativeFieldPath.GetString().Contains(fieldNameStr))
                {
                    propertyEditor->UpdateValue();
                }
            }
        }
    }

    void ObjectEditor::SetSplitRatioInternal(f32 ratio, FSplitBox* excluding)
    {
        for (PropertyEditor* propertyEditor : propertyEditors)
        {
            propertyEditor->SetSplitRatio(ratio, excluding);
        }
    }

    void ObjectEditor::SetEditorGroup(const Array<ObjectEditor*>& group)
    {
        this->editorGroup = group;
    }

    void ObjectEditor::ExpandAll(bool expanded, bool recursively)
    {
        for (PropertyEditor* propertyEditor : propertyEditors)
        {
            propertyEditor->ExpandAll(expanded, recursively);
        }
    }

    ObjectEditor::Self& ObjectEditor::FixedInputWidth(f32 width)
    {
        for (PropertyEditor* propertyEditor : propertyEditors)
        {
            propertyEditor->FixedInputWidth(width);
        }

        return *this;
    }

    f32 ObjectEditor::GetSplitRatio()
    {
        if (propertyEditors.NotEmpty())
        {
            return propertyEditors[0]->GetSplitRatio();
        }
        return 0.35f;
    }

    void ObjectEditor::SetSplitRatio(f32 ratio, FSplitBox* excluding)
    {
        SetSplitRatioInternal(ratio, excluding);

        for (ObjectEditor* editor : editorGroup)
        {
	        if (this != editor)
	        {
                editor->SetSplitRatioInternal(ratio, excluding);
	        }
        }
    }

    void ObjectEditor::ApplySplitRatio(FSplitBox* excluding)
    {
        for (PropertyEditor* propertyEditor : propertyEditors)
        {
            if (propertyEditor->GetSplitBox() != excluding)
            {
                SetSplitRatio(propertyEditor->GetSplitRatio(), excluding);
                return;
            }
        }
    }

    Array<CE::Name> ObjectEditor::FetchSelectedAssetPaths()
    {
        Ref<FWidget> parent = GetParent();

        while (parent != nullptr)
        {
            if (parent->IsOfType<EditorBase>())
            {
                auto editorBase = (EditorBase*)parent.Get();
                return editorBase->GetSelectedAssetPaths();
            }

            parent = parent->GetParent();
        }

        return {};
    }

    void ObjectEditor::BrowseToAssetInAssetBrowser(const CE::Name& fullPath)
    {
        Ref<FWidget> parent = GetParent();

        while (parent != nullptr)
        {
            if (parent->IsOfType<EditorBase>())
            {
                auto editorBase = (EditorBase*)parent.Get();
                editorBase->BrowseToAsset(fullPath);
            }

            parent = parent->GetParent();
        }
    }

    void ObjectEditor::CreateGUI()
    {
        // Default ObjectEditor doesn't support multi-object editing yet.
        if (target == nullptr)
            return;
        
        content->DestroyAllChildren();

        // TODO: Create editors and bind them to respective fields of Object
        // Editor widget will hold reference to FieldType* (edited field) and Array<Object*> (array of target objects)
        // Editor widget will listen to changes made in any of the Array<Object*> through code (NOT UI input) using the Setters.

        struct FieldData
        {
            FieldType* field = nullptr;
            Object* targetObject = nullptr;
        };

        constexpr const char* GeneralCategoryName = "General";

        HashMap<CE::Name, Array<FieldData>> fieldsByCategory;
        Array<CE::Name> categories;
        HashMap<CE::Name, int> categoryOrders;
        HashSet<TypeId> visitedComponents{};

        std::function<void(Object*,bool)> visitor = [&](Object* target, bool recurseComponents)
            {
                Class* clazz = target->GetClass();

                ObjectListener::AddListener(target->GetUuid(), this);
                targetObjectUuids.Add(target->GetUuid());

                u32 numFields = clazz->GetFieldCount();

                for (int i = 0; i < numFields; ++i)
                {
                    Field* field = clazz->GetFieldAt(i);

                    if (!field->IsEditAnywhere() || field->IsHidden())
                        continue;

                    CE::Name category = GeneralCategoryName;

                    if (field->HasAttribute("Category"))
                    {
                        category = field->GetAttribute("Category").GetStringValue();
                        if (!category.IsValid())
                        {
                            category = GeneralCategoryName;
                        }
                    }

                    if (!fieldsByCategory.KeyExists(category))
                    {
                        categories.Add(category);
                    }

                    if (field->HasAttribute("CategoryOrder"))
                    {
                        int order = 0;
                        String::TryParse(field->GetAttribute("CategoryOrder").GetStringValue(), order);
                        categoryOrders[category] = order;
                    }

                    fieldsByCategory[category].Add({ field, target });
                }
                
	            if (target->IsOfType<Actor>())
	            {
                    Actor* actor = static_cast<Actor*>(target);

                    if (actor->GetRootComponent())
                    {
                        visitor(actor->GetRootComponent(), true);
                    }
	            }
            };

        visitor(target, target->IsOfType<Actor>());

        categories.Sort([&](const CE::Name& lhs, const CE::Name& rhs)
            {
                int lhsOrder = categoryOrders[lhs];
                int rhsOrder = categoryOrders[rhs];
                if (lhsOrder < rhsOrder)
                    return true;
                if (lhsOrder == rhsOrder)
                    return String::NaturalCompare(lhs.GetString(), rhs.GetString());
                return false;
            });

        propertyEditors.Clear();

        // Create category sections

        const f32 fontSize = GetDefaults<EditorConfigs>()->GetFontSize();

        for (int i = 0; i < categories.GetSize(); ++i)
        {
            const CE::Name& category = categories[i];
            if (fieldsByCategory[category].IsEmpty())
                continue;

            FExpandableSection* section = nullptr;
            FVerticalStack* expandContent = nullptr;

            FAssignNew(FExpandableSection, section)
            .Title(category.GetString())
            .TitleFontSize(fontSize + 2)
			.ExpandableContent(
                FAssignNew(FVerticalStack, expandContent)
            );

            content->AddChild(section);

            // Create property editors inside this category section

            for (int j = 0; j < fieldsByCategory[category].GetSize(); ++j)
            {
                FieldType* field = fieldsByCategory[category][j].field;
                Object* target = fieldsByCategory[category][j].targetObject;

                if (!field || !target)
                    continue;

                PropertyEditor* propertyEditor = PropertyEditorRegistry::Get()->Create(field, this);
                if (propertyEditor == nullptr)
                    continue;

                propertyEditor->parentObjectEditor = this;
                
                expandContent->AddChild(propertyEditor);

                Array<WeakRef<Object>> targetsArray = { target };

                String fieldName = field->GetName().GetString();

                propertyEditor->InitTarget(targetsArray, fieldName);

                propertyEditors.Add(propertyEditor);
            }

            if (expandContent->GetChildCount() == 0)
            {
                section->BeginDestroy();
            }
        }
    }

} // namespace CE

