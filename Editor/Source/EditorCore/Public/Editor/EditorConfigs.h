#pragma once

namespace CE::Editor
{
    CLASS(Config = Editor)
    class EDITORCORE_API EditorConfigs : public Object
    {
        CE_CLASS(EditorConfigs, Object)
    protected:

        EditorConfigs();
        
    public:

        static constexpr f32 MinFontSize = 6.0f;

        //! @brief Returns the primary font size to use for the editor.
        f32 GetFontSize() const { return Math::Max(MinFontSize, fontSize); }

    private:

        FIELD(Config)
        f32 fontSize = 10;

    };
    
} // namespace CE

#include "EditorConfigs.rtti.h"
