#pragma once

namespace CE::Editor
{

    CLASS()
    class EDITORCORE_API ObjectEditorField : public EditorField, IAssetRegistryListener, IThumbnailSystemListener
    {
        CE_CLASS(ObjectEditorField, EditorField)
    protected:

        ObjectEditorField();

        void Construct() override;

        void OnBind() override;

        void OnBeginDestroy() override;

        void OnAssetRenamed(Uuid bundleUuid, const CE::Name& oldName, const CE::Name& newName, const CE::Name& newPath) override;

        void OnAssetDeleted(const CE::Name& bundlePath) override;

        void OnDirectoryRenamed(const CE::Name& oldPath, const CE::Name& newPath) override;

        void OnThumbnailsUpdated(const Array<CE::Name>& assetPaths) override;

    public: // - Public API -

        bool CanBind(FieldType* field) override;

        bool CanBind(TypeId boundTypeId, TypeId underlyingTypeId) override;

        bool SupportsMouseEvents() const override { return true; }

    protected: // - Internal -

        FUNCTION()
        void OnButtonClicked(FButton* button, Vec2 mousePos);

        void UseSelectedAsset();

        void BrowseToAsset();

        void SelectAsset(AssetData* assetData);

        void UpdateValue() override;

        Ref<FLabel> valueLabel;
        Ref<FStyledWidget> thumbnail;
        Ref<FStyledWidget> colorTag;

        bool registered = false;

        WeakRef<Object> curValue;
        CE::Name curObjectFullPath;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(WeakRef<class ObjectEditor>, ObjectEditorOwner);

        FUSION_WIDGET;
    };
    
}

#include "ObjectEditorField.rtti.h"
