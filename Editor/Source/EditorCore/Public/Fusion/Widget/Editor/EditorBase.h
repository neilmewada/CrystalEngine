#pragma once

namespace CE::Editor
{
    CLASS(Abstract)
    class EDITORCORE_API EditorBase : public EditorMajorDockWindow, IAssetRegistryListener
    {
        CE_CLASS(EditorBase, EditorMajorDockWindow)
    protected:

        EditorBase();

        void Construct() override;

        void HandleEvent(FEvent* event) override;

        void OnBeginDestroy() override;

        void OnAssetRenamed(Uuid bundleUuid, const CE::Name& oldName, const CE::Name& newName, const CE::Name& newPath) override;

    public: // - Public API -

        const Ref<EditorHistory>& GetHistory() const { return history; }

        bool SupportsKeyboardEvents() const override { return true; }

        virtual bool OpenEditor(Ref<Object> targetObject, Ref<Bundle> bundle);

        virtual void OnEditorOpened(Ref<Object> targetObject);

        virtual bool CanEdit(Ref<Object> targetObject) const { return false; }

        virtual ClassType* GetTargetObjectType() const = 0;

        virtual bool AllowMultipleInstances() const { return true; }

        virtual Ref<Object> GetTargetObject() const { return nullptr; }

        virtual Array<CE::Name> GetSelectedAssetPaths() { return {}; }

        virtual void BrowseToAsset(const CE::Name& path) {}

        void SetAssetDirty(bool dirty);

        FUNCTION()
        virtual void SaveChanges();

    protected: // - Internal -

        Ref<EditorHistory> history;
        Ref<Bundle> bundle;
        Uuid bundleUuid;

        bool isAssetDirty = false;
        bool registeredListener = false;

    public: // - Fusion Properties - 

        FUSION_WIDGET;
    };
    
}

#include "EditorBase.rtti.h"
