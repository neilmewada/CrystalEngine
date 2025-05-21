#pragma once

namespace CE::Editor
{
    CLASS(Abstract)
    class EDITORCORE_API EditorBase : public EditorDockTab, IAssetRegistryListener
    {
        CE_CLASS(EditorBase, EditorDockTab)
    protected:

        EditorBase();

        void Construct() override;

        void HandleEvent(FEvent* event) override;

        void OnBeginDestroy() override;

        void OnAssetRenamed(Uuid bundleUuid, const CE::Name& oldName, const CE::Name& newName) override;

    public: // - Public API -

        const Ref<EditorHistory>& GetHistory() const { return history; }

        bool SupportsKeyboardEvents() const override { return true; }

        virtual bool OpenEditor(Ref<Object> targetObject) { return false; }

        virtual void OnEditorOpened(Ref<Object> targetObject);

        virtual bool CanEdit(Ref<Object> targetObject) const { return false; }

        virtual ClassType* GetTargetObjectType() const = 0;

        virtual bool AllowMultipleInstances() const { return true; }

        virtual Ref<Object> GetTargetObject() const { return nullptr; }

        virtual Array<CE::Name> GetSelectedAssetPaths() { return {}; }

        virtual void BrowseToAsset(const CE::Name& path) {}

        void SetAssetDirty(bool dirty);

    protected: // - Internal -

        Ref<EditorHistory> history = nullptr;
        Uuid bundleUuid;

        bool isAssetDirty = false;
        bool registeredListener = false;

    public: // - Fusion Properties - 

        FUSION_WIDGET;
    };
    
}

#include "EditorBase.rtti.h"
