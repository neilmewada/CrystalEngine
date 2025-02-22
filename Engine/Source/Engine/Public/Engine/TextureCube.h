#pragma once

namespace CE
{
    
    CLASS()
    class ENGINE_API TextureCube final : public CE::Texture
    {
        CE_CLASS(TextureCube, CE::Texture)
    public:

        TextureCube();
        virtual ~TextureCube();

        RPI::Texture* GetRpiTexture() override;

        RPI::Texture* CloneRpiTexture() override;

        TextureDimension GetDimension() override { return TextureDimension::TexCube; }

        TextureCube* GetDiffuseConvolution() const { return diffuseConvolution; }

        u32 GetArrayLayers() const override { return 6; }

    private:

        FIELD()
        TextureCube* diffuseConvolution = nullptr;


#if PAL_TRAIT_BUILD_EDITOR
        friend class CE::Editor::TextureAssetImportJob;
#endif
    };

} // namespace CE

#include "TextureCube.rtti.h"
