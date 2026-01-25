#pragma once

namespace CE::Editor
{
    CLASS()
    class CRYSTALEDITOR_API DetailsTab : public EditorDockWindow
    {
        CE_CLASS(DetailsTab, EditorDockWindow)
    protected:

        DetailsTab();

        void Construct() override;
        
    public:

        void SetTargetObject(Ref<Object> object);

    protected:

        Ref<FStyledWidget> editorContainer;
        Ref<ObjectEditor> editor;

        Ref<Object> targetObject;

        FUSION_WIDGET;
    };
    
} // namespace CE

#include "DetailsTab.rtti.h"
