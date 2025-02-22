#pragma once

#include "TextureDefines.h"

namespace CE
{
#if PAL_TRAIT_BUILD_EDITOR
	namespace Editor { class TextureAssetImportJob; }
#endif

	class Texture;
	
	CLASS(Abstract)
	class ENGINE_API Texture : public Asset
	{
		CE_CLASS(Texture, Asset)
	public:

		Texture();
		virtual ~Texture();

		virtual RPI::Texture* GetRpiTexture() = 0;

		virtual RPI::Texture* CloneRpiTexture() = 0;

		virtual TextureDimension GetDimension() = 0;

		CMImageFormat GetCMPixelFormat();

		inline u32 GetWidth() const { return width; }
		inline u32 GetHeight() const { return height; }

		inline u32 GetMipLevels() const { return mipLevels; }

		virtual u32 GetArrayLayers() const = 0;

		inline CE::TextureFormat GetPixelFormat() const { return pixelFormat; }

	protected:

		RPI::Texture* rpiTexture = nullptr;

		FIELD()
		BinaryBlob source{};

		FIELD()
		CE::TextureFormat pixelFormat = CE::TextureFormat::None;

		FIELD(ImportSetting)
		TextureCompressionQuality compressionQuality = TextureCompressionQuality::Default;

		FIELD(ImportSetting)
		TextureDimension dimension = TextureDimension::None;

		FIELD()
		TextureSourceCompressionFormat sourceCompressionFormat = TextureSourceCompressionFormat::None;

		FIELD()
		RHI::FilterMode filter = RHI::FilterMode::Linear;

		FIELD()
		TextureAddressMode addressModeU = TextureAddressMode::Repeat;

		FIELD()
		TextureAddressMode addressModeV = TextureAddressMode::Repeat;

		FIELD()
		RHI::SamplerBorderColor borderColor = RHI::SamplerBorderColor::FloatTransparentBlack;

		FIELD()
		u8 anisoLevel = 0;

		FIELD()
		u32 width = 0;

		FIELD()
		u32 height = 0;

		FIELD()
		u32 mipLevels = 1;

#if PAL_TRAIT_BUILD_EDITOR
		friend class CE::Editor::TextureAssetImportJob;
#endif
	};
    
} // namespace CE

#include "Texture.rtti.h"
