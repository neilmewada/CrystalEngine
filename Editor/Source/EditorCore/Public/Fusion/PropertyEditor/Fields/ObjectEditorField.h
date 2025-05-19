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

        void UpdateValue() override;

    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "ObjectEditorField.rtti.h"
