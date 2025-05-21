#pragma once

namespace CE::Editor
{
    class PropertyEditor;
    class EditorHistory;

    CLASS()
    class EDITORCORE_API ObjectEditor : public FStyledWidget, IObjectUpdateListener
    {
        CE_CLASS(ObjectEditor, FStyledWidget)
    protected:

        ObjectEditor();

        virtual ~ObjectEditor();

        void Construct() override;

        void OnBeginDestroy() override;

        void OnObjectFieldChanged(Uuid objectUuid, const CE::Name& fieldName) override;

        void SetSplitRatioInternal(f32 ratio, FSplitBox* excluding = nullptr);

    public:

        void SetEditorGroup(const Array<ObjectEditor*>& group);

        virtual bool SupportsMultiObjectEditing() const { return false; }

        void ExpandAll(bool expanded, bool recursively = false);

        Self& FixedInputWidth(f32 width);

        f32 GetSplitRatio();

        void SetSplitRatio(f32 ratio, FSplitBox* excluding = nullptr);

        void ApplySplitRatio(FSplitBox* excluding = nullptr);

        void SetEditorHistory(const Ref<EditorHistory>& history);

        Ref<EditorHistory> GetEditorHistory() const { return history.Lock(); }

        Array<CE::Name> FetchSelectedAssetPaths();

        void BrowseToAssetInAssetBrowser(const CE::Name& fullPath);

    protected:

        virtual void CreateGUI();

        FVerticalStack* content = nullptr;

        Array<PropertyEditor*> propertyEditors;

        FIELD(ReadOnly)
        Array<Object*> targets;

        FIELD(ReadOnly)
        Object* target = nullptr;

        WeakRef<EditorHistory> history = nullptr;

        Array<ObjectEditor*> editorGroup;
        HashSet<Uuid> targetObjectUuids;

    public:

        friend class ObjectEditorRegistry;
    };
    
} // namespace CE

#include "ObjectEditor.rtti.h"
