#pragma once

namespace CE
{
	CLASS()
	class FUSIONCORE_API FFontManager : public Object
	{
		CE_CLASS(FFontManager, Object)
	public:

		FFontManager();

		void Init();
		void Shutdown();

		const Name& GetDefaultFontFamily() const;

		u32 GetDefaultFontSize() const;

		FFontAtlas* GetDefaultFontAtlas();

		bool RegisterFont(const Name& fontName, const Array<CharRange>& characterSets, 
			Stream* regularFontFile, Stream* italicFontFile = nullptr, Stream* boldFontFile = nullptr, Stream* boldItalicFontFile = nullptr);

		bool RegisterSDFFont(const Name& fontName, const Array<CharRange>& characterSets,
			Stream* regularFontFile, Stream* italicFontFile = nullptr, Stream* boldFontFile = nullptr, Stream* boldItalicFontFile = nullptr);

		bool DeregisterFont(const Name& fontName);

		FFontAtlas* FindFont(const Name& fontName);
		Ref<FSDFFontAtlas> FindSDFFont(const Name& fontName);

		template<typename Func>
		void IterateFontAtlases(const Func& func)
		{
			for (const auto& [fontFamily, fontAtlas] : fontAtlases)
			{
				func(fontAtlas);
			}
		}

		u32 GetFontAtlasCount() const { return fontAtlases.GetSize(); }

		//! @brief Flushes all the changes to GPU
		void Flush(u32 imageIndex);

	private:

		bool LoadFontFace(Stream* ttfFile, FT_Face& outFace, u8** outData);

		HashMap<Name, FFontAtlas*> fontAtlases;
		HashMap<Name, Ref<FSDFFontAtlas>> sdfFontAtlases;

		FT_Library ft = nullptr;

		friend class FFontAtlas;
		friend class FSDFFontAtlas;
	};

} // namespace CE

#include "FFontManager.rtti.h"