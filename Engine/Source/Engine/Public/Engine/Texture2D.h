#pragma once

namespace CE
{
#if PAL_TRAIT_BUILD_EDITOR
	namespace Editor { class TextureAssetImportJob; class FontAssetImportJob; }
#endif

	CLASS()
	class ENGINE_API Texture2D : public CE::Texture
	{
		CE_CLASS(Texture2D, CE::Texture)
	public:

		Texture2D();

		RPI::Texture* GetRpiTexture() override;

		RPI::Texture* CloneRpiTexture() override;

		TextureDimension GetDimension() override
		{
			return TextureDimension::Tex2D;
		}

		u32 GetArrayLayers() const override
		{
			return arrayCount;
		}

		CMImage GetCMImage();

		static Ref<CE::Texture2D> Create(Ref<Object> outer, const String& name, const CMImage& sourceImage, 
			TextureAddressMode addressModeU = TextureAddressMode::ClampToBorder, TextureAddressMode addressModeV = TextureAddressMode::ClampToBorder);

	protected:

		FIELD()
		TextureColorSpace colorSpace = TextureColorSpace::None;

		FIELD()
		u32 arrayCount = 1;

	public:

		CE_PROPERTY(ColorSpace, colorSpace);

#if PAL_TRAIT_BUILD_EDITOR
		friend class CE::Editor::TextureAssetImportJob;
		friend class CE::Editor::FontAssetImportJob;
#endif
	};
    
} // namespace CE

#include "Texture2D.rtti.h"
