#pragma once

namespace CE
{

    CLASS()
    class ENGINE_API Font : public Asset
    {
        CE_CLASS(Font, Asset)
    public:

        virtual ~Font();

        inline RPI::FontAtlasAsset* GetAtlasData() const { return atlasAsset; }

    private:

        void OnAfterDeserialize() override;

        FIELD()
        RPI::FontAtlasAsset* atlasAsset = nullptr;

#if PAL_TRAIT_BUILD_EDITOR
        friend class CE::Editor::FontAssetImportJob;
#endif
    };

} // namespace CE

#include "Font.rtti.h"