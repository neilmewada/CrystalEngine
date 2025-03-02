#pragma once

namespace CE
{
    CLASS()
    class CRYSTALEDITOR_API CrystalEditorStyle : public EditorStyle
    {
        CE_CLASS(CrystalEditorStyle, EditorStyle)
    protected:

        CrystalEditorStyle();
        
    public:

        virtual ~CrystalEditorStyle();

        void InitEditorStyle() override;

        Ref<AssetBrowserItemStyle> assetBrowserItem;

    };
    
} // namespace CE

#include "CrystalEditorStyle.rtti.h"
