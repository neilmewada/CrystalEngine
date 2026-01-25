#include "CrystalEditor.h"

namespace CE::Editor
{

    DataAssetEditor::DataAssetEditor()
    {

    }

    void DataAssetEditor::Construct()
    {
	    Super::Construct();

        Title("DataAsset Editor");

        ToolBarEnabled(true);
        MenuBarEnabled(false);

        toolBar->Content(
            EditorToolBar::NewImageButton("/Editor/Assets/Icons/Save")
            .OnClicked(FUNCTION_BINDING(this, SaveChanges))
        );

        minorDockspace->AddDockWindow(
            FAssignNew(DetailsTab, detailsTab)
        );

        detailsTab->SetOwnerEditor(this);
    }

    ClassType* DataAssetEditor::GetTargetObjectType() const
    {
        return DataAsset::StaticClass();
    }

    bool DataAssetEditor::CanEdit(Ref<Object> targetObject) const
    {
        return targetObject.IsValid() && targetObject->IsOfType<CE::DataAsset>();
    }

    bool DataAssetEditor::OpenEditor(Ref<Object> targetObject, Ref<Bundle> bundle)
    {
        if (!targetObject)
            return false;
        if (!targetObject->IsOfType<CE::DataAsset>())
            return false;

        Super::OpenEditor(targetObject, bundle);

        Ref<CE::DataAsset> dataAsset = Object::CastTo<CE::DataAsset>(targetObject);
        if (this->targetAsset == dataAsset)
            return true;

        this->targetAsset = dataAsset;
        this->bundle = bundle;
        SetDataAsset(dataAsset);

        return true;
    }

    void DataAssetEditor::SetDataAsset(Ref<DataAsset> dataAsset)
    {
        if (dataAsset.IsNull())
            return;

    	String title = dataAsset->GetName().GetString();
        if (Ref<Bundle> bundle = dataAsset->GetBundle())
        {
            title = bundle->GetName().GetString();
        }

        Title(title);

        //editorContainer->RemoveChildWidget();

        if (editor.IsValid())
        {
            editor->BeginDestroy();
            editor = nullptr;
        }

        detailsTab->SetTargetObject(dataAsset);

        //editor = ObjectEditorRegistry::Get().Create(dataAsset.Get(), GetHistory());
        //editor->ExpandAll(true, true);
        //editorContainer->AddChild(editor.Get());
    }

} // namespace CE

