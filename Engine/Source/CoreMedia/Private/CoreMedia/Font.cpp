#include "CoreMedia.h"

#include "stb_image_write.h"

#undef DLL_EXPORT
#define FT_PUBLIC_FUNCTION_ATTRIBUTE

#include <ft2build.h>
#include "freetype/freetype.h"

namespace CE
{
	CMFontAtlas::~CMFontAtlas()
	{
		atlas.Free();
	}

	CMFontAtlas* CMFontAtlas::GenerateFromFontFile(const IO::Path& filePath, const CMFontAtlasGenerateInfo& generateInfo)
	{
		const String filePathName = filePath.GetString();

		FT_Library ft;
		FT_Face    face;

		u32 fontSize = generateInfo.fontSize;

		FT_Init_FreeType(&ft);
		if (FT_New_Face(ft, filePathName.GetCString(), 0, &face))
		{
			FT_Done_FreeType(ft);
			return nullptr;
		}

		CMFontAtlas* fontAtlas = new CMFontAtlas();
		Array<CMGlyphInfo>& info = fontAtlas->glyphInfos;

		Array<FT_ULong> charSet{};
		for (auto rangeInfo : generateInfo.charSetRanges)
		{
			if (rangeInfo.charCode != 0)
			{
				charSet.Add(rangeInfo.charCode);
			}
			else if (rangeInfo.range.min <= rangeInfo.range.max)
			{
				//int idx = charSet.GetSize();
				//charSet.Resize(charSet.GetSize() + rangeInfo.range.max - rangeInfo.range.min + 1);
				for (FT_ULong c = rangeInfo.range.min; c <= rangeInfo.range.max; c++)
				{
					charSet.Add(c);
					//charSet[idx++] = c;
				}
			}
		}

		const int padding = generateInfo.padding;

		const u32 numGlyphs = charSet.GetSize();
		info.Resize(numGlyphs);

		//FT_Set_Char_Size(face, 0, fontSize << 6, 96, 96);
		FT_Set_Pixel_Sizes(face, 0, fontSize);

		int max_dim = (int)((f32)(1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(numGlyphs)));
		int tex_width = 1;
		while (tex_width < max_dim) tex_width <<= 1;
		int tex_height = tex_width;

		// render glyphs to atlas

		char* pixels = (char*)calloc(tex_width * tex_height, 1);

		int pen_x = generateInfo.startOffsetX + padding * 2;
		int pen_y = generateInfo.startOffsetY + padding * 2;
		
		for (int i = 0; i < numGlyphs; ++i) {
			FT_ULong charCode = charSet[i];
			FT_Load_Char(face, charCode, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
			FT_Bitmap* bmp = &face->glyph->bitmap;

			if (pen_x + bmp->width >= tex_width - padding * 2) {
				pen_x = padding * 2;
				pen_y += ((face->size->metrics.height >> 6) + 1) + padding;
			}

			for (int row = 0; row < bmp->rows; ++row) 
			{
				for (int col = 0; col < bmp->width; ++col) 
				{
					int x = pen_x + col;
					int y = pen_y + row;
					pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
				}
			}

			// this is stuff you'd need when rendering individual glyphs out of the atlas

			info[i].charCode = (u32)charCode;
			info[i].x0 = pen_x;
			info[i].y0 = pen_y;
			info[i].x1 = pen_x + bmp->width;
			info[i].y1 = pen_y + bmp->rows;
			info[i].width = bmp->width;
			info[i].height = bmp->rows;

			info[i].xOffset = face->glyph->bitmap_left;
			info[i].yOffset = face->glyph->bitmap_top;
			info[i].advance = face->glyph->advance.x >> 6;
			info[i].fontSize = fontSize;

			pen_x += bmp->width + 1;
			pen_x += padding;
		}

		f32 unitsPerEM = face->units_per_EM;
		f32 scaleFactor = (f32)fontSize / unitsPerEM;

		fontAtlas->metrics.ascender = face->ascender * scaleFactor;
		fontAtlas->metrics.descender = face->descender * scaleFactor;

		fontAtlas->metrics.lineGap = (face->height - (face->ascender - face->descender)) * scaleFactor;
		fontAtlas->metrics.lineHeight = (face->ascender - face->descender + fontAtlas->metrics.lineGap) * scaleFactor;

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		//String outPath = (PlatformDirectories::GetLaunchDir() / "Temp/font_output.png").GetString();
		//stbi_write_png(outPath.GetCString(), tex_width, tex_height, 1, pixels, tex_width);

		fontAtlas->atlas.allocated = true;
		fontAtlas->atlas.data = pixels;
		fontAtlas->atlas.width = tex_width;
		fontAtlas->atlas.height = tex_height;
		fontAtlas->atlas.bitsPerPixel = 8;
		fontAtlas->atlas.numChannels = 1;
		fontAtlas->atlas.bitDepth = 8;
		fontAtlas->atlas.format = CMImageFormat::R8;
		fontAtlas->atlas.sourceFormat = CMImageSourceFormat::None;

		return fontAtlas;
	}

} // namespace CE
