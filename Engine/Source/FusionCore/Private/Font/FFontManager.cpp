#include "FusionCore.h"

#undef DLL_EXPORT
#define FT_PUBLIC_FUNCTION_ATTRIBUTE

#include <ft2build.h>
#include "freetype/freetype.h"

#include "FontConstants.inl"

namespace CE
{

    FFontManager::FFontManager()
    {
        
    }

    void FFontManager::Init()
    {
        FT_Init_FreeType(&ft);

        Array<CharRange> englishCharSets{};
        englishCharSets.Add(CharRange('a', 'z'));
        englishCharSets.Add(CharRange('A', 'Z'));
        englishCharSets.Add(CharRange('0', '9'));
        englishCharSets.Add(CharRange(32, 47));
        englishCharSets.Add(CharRange(58, 64));
        englishCharSets.Add(CharRange(91, 96));
        englishCharSets.Add(CharRange(123, 126));

        IO::Path fontDirectory = PlatformDirectories::GetLaunchDir() / "Engine/Resources/Fonts";
        IO::Path robotoRegular = fontDirectory / "Roboto-Regular.ttf";
        IO::Path robotoItalic = fontDirectory / "Roboto-Italic.ttf";
        IO::Path robotoBold = fontDirectory / "Roboto-Bold.ttf";
        IO::Path robotoBoldItalic = fontDirectory / "Roboto-BoldItalic.ttf";

        FileStream regular = FileStream(robotoRegular, Stream::Permissions::ReadOnly);
        FileStream italic = FileStream(robotoItalic, Stream::Permissions::ReadOnly);
        FileStream bold = FileStream(robotoBold, Stream::Permissions::ReadOnly);
        FileStream boldItalic = FileStream(robotoBoldItalic, Stream::Permissions::ReadOnly);

        RegisterFont(GetDefaultFontFamily(), englishCharSets, &regular, &italic, &bold, &boldItalic);

        regular.Seek(0, SeekMode::Begin);
        italic.Seek(0, SeekMode::Begin);
        bold.Seek(0, SeekMode::Begin);
        boldItalic.Seek(0, SeekMode::Begin);

        RegisterSDFFont(GetDefaultFontFamily(), englishCharSets, &regular, &italic, &bold, &boldItalic);
    }

    void FFontManager::Shutdown()
    {
        auto deleteFace = [](FT_Face& face, u8*& data)
            {
                if (face != nullptr)
                {
                    FT_Done_Face(face);
                    face = nullptr;
                }
	            if (data != nullptr)
	            {
                    delete[] data;
                    data = nullptr;
	            }
            };

        for (auto [fontName, atlas] : fontAtlases)
        {
            deleteFace(atlas->regular, atlas->regularData);
            deleteFace(atlas->italic, atlas->italicData);
            deleteFace(atlas->bold, atlas->boldData);
            deleteFace(atlas->boldItalic, atlas->boldItalicData);
	        
            atlas->ft = nullptr;

            atlas->BeginDestroy();
        }

        for (auto [fontName, atlas] : sdfFontAtlases)
        {
            deleteFace(atlas->regular, atlas->regularData);
            deleteFace(atlas->italic, atlas->italicData);
            deleteFace(atlas->bold, atlas->boldData);
            deleteFace(atlas->boldItalic, atlas->boldItalicData);

			atlas->ft = nullptr;

			atlas->BeginDestroy();
        }

        FT_Done_FreeType(ft);
        ft = nullptr;

        fontAtlases.Clear();
        sdfFontAtlases.Clear();
    }

    const Name& FFontManager::GetDefaultFontFamily() const
    {
        thread_local const CE::Name robotoFamily = "Roboto";
        return robotoFamily;
    }

    u32 FFontManager::GetDefaultFontSize() const
    {
        return DefaultFontSize;
    }

    FFontAtlas* FFontManager::GetDefaultFontAtlas()
    {
        return FindFont(GetDefaultFontFamily());
    }

    bool FFontManager::LoadFontFace(Stream* ttfFile, FT_Face& outFace, u8** outData)
    {
        if (ttfFile == nullptr || !ttfFile->CanRead() || ttfFile->GetLength() == 0)
            return false;

        *outData = new u8[ttfFile->GetLength()];

        ttfFile->Read(*outData, ttfFile->GetLength());

        if (FT_New_Memory_Face(ft, *outData, ttfFile->GetLength(), 0, &outFace))
        {
	        return false;
        }

        return true;
    }

    bool FFontManager::RegisterFont(const Name& fontName, const Array<CharRange>& characterSets,
        Stream* regularFontFile, Stream* italicFontFile, Stream* boldFontFile, Stream* boldItalicFontFile)
    {
        ZoneScoped;

        if (!fontName.IsValid() || characterSets.IsEmpty() || regularFontFile == nullptr ||
            !regularFontFile->CanRead() || regularFontFile->GetLength() == 0)
            return false;

        if (fontAtlases.KeyExists(fontName) && fontAtlases[fontName] != nullptr)
        {
            return true;
        }

        FT_Face face;
        u8* faceData = nullptr;

        if (!LoadFontFace(regularFontFile, face, &faceData))
        {
	        return false;
        }

        FFontAtlas* fontAtlas = CreateObject<FFontAtlas>(this, fontName.GetString());

        LoadFontFace(italicFontFile, fontAtlas->italic, &fontAtlas->italicData);
        LoadFontFace(boldFontFile, fontAtlas->bold, &fontAtlas->boldData);
        LoadFontFace(boldItalicFontFile, fontAtlas->boldItalic, &fontAtlas->boldItalicData);

        fontAtlas->ft = ft;
        fontAtlas->regular = face;
        fontAtlas->regularData = faceData;

        Array<u32> charSet{};
        charSet.Reserve(256);
        for (auto rangeInfo : characterSets)
        {
            if (rangeInfo.charCode != 0)
            {
                charSet.Add(rangeInfo.charCode);
            }
            else if (rangeInfo.range.min <= rangeInfo.range.max)
            {
                for (FT_ULong c = rangeInfo.range.min; c <= rangeInfo.range.max; c++)
                {
                    charSet.Add(c);
                }
            }
        }

        fontAtlas->Init(charSet);

        fontAtlases[fontName] = fontAtlas;

        return true;
    }

    bool FFontManager::RegisterSDFFont(const Name& fontName, const Array<CharRange>& characterSets,
        Stream* regularFontFile, Stream* italicFontFile, Stream* boldFontFile, Stream* boldItalicFontFile)
    {
        ZoneScoped;

        if (!fontName.IsValid() || characterSets.IsEmpty() || regularFontFile == nullptr ||
            !regularFontFile->CanRead() || regularFontFile->GetLength() == 0)
            return false;

        if (sdfFontAtlases.KeyExists(fontName) && sdfFontAtlases[fontName] != nullptr)
        {
            return true;
		}

        FT_Face face;
        u8* faceData = nullptr;

        if (!LoadFontFace(regularFontFile, face, &faceData))
        {
            return false;
        }

		Ref<FSDFFontAtlas> fontAtlas = CreateObject<FSDFFontAtlas>(this, fontName.GetString());

        LoadFontFace(italicFontFile, fontAtlas->italic, &fontAtlas->italicData);
        LoadFontFace(boldFontFile, fontAtlas->bold, &fontAtlas->boldData);
        LoadFontFace(boldItalicFontFile, fontAtlas->boldItalic, &fontAtlas->boldItalicData);

        fontAtlas->ft = ft;
        fontAtlas->regular = face;
        fontAtlas->regularData = faceData;

        Array<u32> charSet{};
        charSet.Reserve(256);
        for (auto rangeInfo : characterSets)
        {
            if (rangeInfo.charCode != 0)
            {
                charSet.Add(rangeInfo.charCode);
            }
            else if (rangeInfo.range.min <= rangeInfo.range.max)
            {
                for (FT_ULong c = rangeInfo.range.min; c <= rangeInfo.range.max; c++)
                {
                    charSet.Add(c);
                }
            }
        }

        FSDFFontAtlasInitInfo initInfo{};
        initInfo.sdfGlyphShader = FusionApplication::Get()->sdfGlyphShader;
		initInfo.fontName = fontAtlas->GetName().GetString();
		initInfo.characterSet = charSet;

        fontAtlas->Init(initInfo);

		sdfFontAtlases[fontName] = fontAtlas;

        return true;
    }

    bool FFontManager::DeregisterFont(const Name& fontName)
    {
        ZoneScoped;

        if (!fontAtlases.KeyExists(fontName))
            return false;

        FFontAtlas* font = fontAtlases[fontName];
        fontAtlases.Remove(fontName);

        if (font)
        {
            font->BeginDestroy();
            return true;
        }

        return false;
    }

    FFontAtlas* FFontManager::FindFont(const Name& fontName)
    {
        if (!fontAtlases.KeyExists(fontName))
            return nullptr;

        return fontAtlases[fontName];
    }

    Ref<FSDFFontAtlas> FFontManager::FindSDFFont(const Name& fontName)
    {
        if (!sdfFontAtlases.KeyExists(fontName))
            return nullptr;

        return sdfFontAtlases[fontName];
    }

    void FFontManager::Flush(u32 imageIndex)
    {
        ZoneScoped;

        for (auto [fontName, atlas] : fontAtlases)
        {
            atlas->Flush(imageIndex);
        }

        for (auto [fontName, atlas] : sdfFontAtlases)
        {
            atlas->Flush(imageIndex);
        }
    }

} // namespace CE
