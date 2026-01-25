#pragma once

namespace CE::Editor
{
    CLASS()
    class EDITORCORE_API EditorToolBar : public FStyledWidget
    {
        CE_CLASS(EditorToolBar, FStyledWidget)
    public:

        // - Public API -

        static FImageButton& NewImageButton(const CE::Name& imagePath);
        static FWidget& NewSeparator();

    protected:

        EditorToolBar();

        void Construct() override;

        Ref<FStackBox> contentBox;

    public: // - Fusion Properties - 

        template<typename... TArgs> requires TValidate_Children<TArgs...>::Value and (sizeof...(TArgs) > 0)
        Self& Content(const TArgs&... childWidget)
        {
            if (contentBox)
            {
                contentBox->operator()(childWidget...);
            }
            return *this;
        }

        FUSION_WIDGET;
    };
    
}

#include "EditorToolBar.rtti.h"
