#pragma once

namespace CE
{
    CLASS()
    class EDITORCORE_API EditorDockspaceStyle : public FDockspaceStyle
    {
        CE_CLASS(EditorDockspaceStyle, FDockspaceStyle)
    public:

        virtual ~EditorDockspaceStyle();

    protected:

        EditorDockspaceStyle();

        SubClass<FWidget> GetWidgetClass() const override;

        void MakeStyle(FWidget& widget) override;

    public:

        

    };
    
} // namespace CE

#include "EditorDockspaceStyle.rtti.h"
