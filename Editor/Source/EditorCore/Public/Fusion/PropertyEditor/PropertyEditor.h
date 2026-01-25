#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API PropertyEditor : public FCompoundWidget
    {
        CE_CLASS(PropertyEditor, FCompoundWidget)
    protected:

        PropertyEditor();

        void Construct() override;

        void ConstructDefaultEditor();

        virtual void ConstructEditor();

        void OnPaint(FPainter* painter) override;

    public: // - Public API -

        virtual void InitTarget(const Array<WeakRef<Object>>& targets, const String& relativeFieldPath);

        virtual void UpdateTarget(const Array<WeakRef<Object>>& targets, const String& relativeFieldPath);

		const CE::Name& GetFieldName() const { return fieldName; }
		const CE::Name& GetRelativeFieldPath() const { return relativeFieldPath; }

        FHorizontalStack* GetLeft() const { return left; }
        FHorizontalStack* GetRight() const { return right; }

        virtual bool IsFieldSupported(FieldType* field) const;

        virtual bool IsFieldSupported(TypeId fieldTypeId) const;

        virtual bool IsExpandable();

        virtual void UpdateValue();

        bool IsExpanded() const { return isExpanded; }

        int GetIndentationLevel() const { return indentation; }

        void SetIndentationLevel(int value);

        FSplitBox* GetSplitBox() const { return splitBox; }

		EditorField* GetEditorField() const { return editorField; }

        f32 GetSplitRatio() const;

        void SetSplitRatio(f32 ratio);

        virtual void SetSplitRatio(f32 ratio, FSplitBox* excluding);

        virtual PropertyEditor& FixedInputWidth(f32 width);

        //! @brief This function will be called on the class' CDI to check if this property editor
        //! supports multi object editing.
        virtual bool SupportsMultiObjectEditing() const { return false; }


        virtual void ExpandAll(bool expanded, bool recursively = false);

        FUNCTION()
        void ToggleExpansion();

    protected: // - Internal -

        void ParseShowIfCondition(Ptr<FieldType> field, Ref<Object> target, const String& parentFieldRelativePath);

        void ValidateShowIfCondition(Ref<Object> target, const String& parentFieldRelativePath);

        void UpdateExpansion();

        virtual void OnExpand() {}
        virtual void OnCollapse() {}

        struct ShowCondition
        {
            bool valid = false;
            bool isEquality = false;
            String comparisonFieldName;
            String showIfValue;
        };

        bool isExpanded = false;
        int indentation = 0;
        TypeInfo* fieldDeclType = nullptr;

        FSplitBox* splitBox = nullptr;

        FVerticalStack* contentStack = nullptr;
        FVerticalStack* expansionStack = nullptr;

        FImageButton* expansionArrow = nullptr;
        FHorizontalStack* left = nullptr;
        FHorizontalStack* right = nullptr;

        FLabel* fieldNameLabel = nullptr;
        EditorField* editorField = nullptr;
        ObjectEditor* objectEditor = nullptr;

        String cachedElementNameFormat = "Index {}";

        ShowCondition showCondition{};

        //FieldType* field = nullptr;
        CE::Name fieldName;
        CE::Name relativeFieldPath;
        Array<WeakRef<Object>> targets;
        WeakRef<Object> target = nullptr;

        Array<Ref<PropertyEditor>> structProperties;
        WeakRef<PropertyEditor> parentEditor;
        WeakRef<ObjectEditor> parentObjectEditor;

        bool hasStructTypeNameOverride = false;
        CE::Name structTypeNameOverride;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Text, fieldNameLabel, FieldNameText);

        FUSION_PROPERTY(FBrush, HoverRowBackground);

        FUSION_WIDGET;
        friend class PropertyEditorRegistry;
        friend class ObjectEditor;
        friend class ArrayPropertyEditor;
    };
    
}

#include "PropertyEditor.rtti.h"
