#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API DataAssetEditor : public EditorBase
    {
        CE_CLASS(DataAssetEditor, EditorBase)
    protected:

        DataAssetEditor();

        void Construct() override;

    public: // - Public API -

        ClassType* GetTargetObjectType() const override;

        bool CanEdit(Ref<Object> targetObject) const override;

        bool OpenEditor(Ref<Object> targetObject, Ref<Bundle> bundle) override;

        Ref<Object> GetTargetObject() const override { return targetAsset; }

    private: // - Internal -

        void SetDataAsset(Ref<DataAsset> dataAsset);

        Ref<CE::DataAsset> targetAsset;
        Ref<ObjectEditor> editor;
        Ref<DetailsTab> detailsTab;

        Ref<FStyledWidget> editorContainer;
    };
    
} // namespace CE

#include "DataAssetEditor.rtti.h"
