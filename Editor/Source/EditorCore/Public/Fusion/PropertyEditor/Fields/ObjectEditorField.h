#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API ObjectEditorField : public EditorField
    {
        CE_CLASS(ObjectEditorField, EditorField)
    protected:

        ObjectEditorField();

        void Construct() override;

    public: // - Public API -

        bool CanBind(FieldType* field) override;

        bool CanBind(TypeId boundTypeId, TypeId underlyingTypeId) override;

        bool SupportsMouseEvents() const override { return true; }

    protected: // - Internal -

        FUNCTION()
        void OnButtonClicked(FButton* button, Vec2 mousePos);

        void SelectAsset(AssetData* assetData);

        void UpdateValue() override;

        Ref<FLabel> valueLabel;
        Ref<FLabel> pathLabel;

        WeakRef<Object> curValue;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "ObjectEditorField.rtti.h"
