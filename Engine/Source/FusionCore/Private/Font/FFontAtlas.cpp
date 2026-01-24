#include "FusionCore.h"

#include "stb_image_write.h"

#undef DLL_EXPORT
#define FT_PUBLIC_FUNCTION_ATTRIBUTE

#include <ft2build.h>
#include "freetype/freetype.h"

#include "FontConstants.inl"

namespace CE
{
    static constexpr f32 FontScaling = 1.0f;

    FFontAtlas::FFontAtlas()
    {
        
    }

    void FFontAtlas::Init(const Array<u32>& charSet)
    {
        ZoneScoped;

        if (atlasImageMips.NotEmpty())
            return;
        if (regular == nullptr)
            return;

        int numFrames = RHI::FrameScheduler::Get()->GetFramesInFlight();

        glyphBuffer.Init("GlyphBuffer", GlyphBufferInitialCount, numFrames);

        f32 unitsPerEM = regular->units_per_EM;
        f32 scaleFactor = 1.0f / unitsPerEM; // 1.0f is used as a font size here

        metrics.ascender = regular->ascender * scaleFactor;
        metrics.descender = regular->descender * scaleFactor;
        metrics.lineGap = (regular->height - (regular->ascender - regular->descender)) * scaleFactor;
        metrics.lineHeight = (regular->ascender - regular->descender + metrics.lineGap) * scaleFactor;

        Ptr<FAtlasImage> atlas = new FAtlasImage;
        atlas->ptr = new u8[FontAtlasSize * FontAtlasSize];
        memset(atlas->ptr, 0, FontAtlasSize * FontAtlasSize);
        atlas->atlasSize = FontAtlasSize;

        atlasImageMips.Add(atlas);

        Array<String> pages = GetPages();
        pages.Add(String::Format("Page {}", pages.GetSize()));
        SetPages(pages);

        AddGlyphs(charSet, DefaultFontSize);

        Array<CMImage> images;

        int idx = 0;
        for (FAtlasImage* atlasImageMip : atlasImageMips)
        {
            CMImage image = CMImage::LoadRawImageFromMemory(atlasImageMip->ptr, atlasImageMip->atlasSize, atlasImageMip->atlasSize,
                CMImageFormat::R8, CMImageSourceFormat::None, 8, 8);

#if PAL_TRAIT_BUILD_EDITOR
            image.EncodeToPNG(PlatformDirectories::GetLaunchDir() / String::Format("Temp/FontAtlas{}.png", idx++));
#endif

            images.Add(image);
        }

        RHI::SamplerDescriptor fontSampler{};
        fontSampler.addressModeU = fontSampler.addressModeV = fontSampler.addressModeW = RHI::SamplerAddressMode::ClampToBorder;
        fontSampler.borderColor = RHI::SamplerBorderColor::FloatTransparentBlack;
        fontSampler.enableAnisotropy = false;
        fontSampler.samplerFilterMode = RHI::FilterMode::Linear;
        //fontSampler.samplerFilterMode = FilterMode::Nearest;

        atlasTexture = new RPI::Texture(images, fontSampler);

        RPI::Shader* fusionShader = FusionApplication::Get()->GetFusionShader();
        RPI::Shader* fusionShader2 = FusionApplication::Get()->GetFusionShader2();

        RHI::ShaderResourceGroupDescriptor fontSrgDesc{};
        fontSrgDesc.name = "Font SRG (SRG_PerMaterial)";
        fontSrgDesc.layout = fusionShader2->GetDefaultVariant()->GetSrgLayout(RHI::SRGType::PerMaterial);

        fontSrg2 = RHI::gDynamicRHI->CreateShaderResourceGroup(fontSrgDesc);

        fontSrg2->Bind("_FontAtlas", atlasTexture->GetRhiTexture());
        fontSrg2->Bind("_FontAtlasSampler", atlasTexture->GetSamplerState());

        fontSrg2->FlushBindings();
    }

    u32 FFontAtlas::GetAtlasSize() const
    {
        return FontAtlasSize;
    }

    FFontGlyphInfo FFontAtlas::FindOrAddGlyph(u32 charCode, u32 fontSize, bool isBold, bool isItalic)
    {
        ZoneScoped;
        char __text[2] = { (char)charCode, 0 };
        ZoneText(__text, 1);

        u32 fontSizeInAtlas = fontSize;

        // Find the closest matching font size

	    {
            ZoneNamedN(__fontSizes, "_FontSizeLoop", true);

		    for (int i = 0; i < gFontSizes.GetSize(); ++i)
		    {
		    	if (gFontSizes[i] == fontSize || 
					(i == 0 && fontSize <= gFontSizes[i]) || 
					(i == gFontSizes.GetSize() - 1 && fontSize >= gFontSizes[i]))
		    	{
		    		fontSizeInAtlas = gFontSizes[i];
		    		break;
		    	}

                // This was modified for Windows OS
		    	if (gFontSizes[i] < fontSize && fontSize < gFontSizes[i + 1])
		    	{
		    		int splitSize = Math::RoundToInt(gFontSizes[i] + (gFontSizes[i + 1] - gFontSizes[i]) * 0.1f);
		    		if (fontSize <= gFontSizes[i])
		    			fontSizeInAtlas = gFontSizes[i];
		    		else
		    			fontSizeInAtlas = gFontSizes[i + 1];
		    		break;
		    	}
		    }
	    }
        
	    {
            ZoneNamedN(__hashMaps, "_HashMaps", true);

		    if (!mipIndicesByCharacter.KeyExists({ charCode, fontSizeInAtlas }))
		    {
		    	static Array<u32> charSet{};
		    	charSet.Resize(1);
		    	charSet[0] = charCode;
		    	AddGlyphs(charSet, fontSizeInAtlas, isBold, isItalic);
		    }

        	int mipIndex = mipIndicesByCharacter[{ charCode, fontSizeInAtlas }];
        	Ptr<FAtlasImage> atlasMip = atlasImageMips[mipIndex];

        	auto& glyphsByFontSize = atlasMip->glyphsByFontSize[fontSizeInAtlas];

        	if (!glyphsByFontSize.KeyExists(charCode))
        	{
        		static Array<u32> charSet{};
        		charSet.Resize(1);
        		charSet[0] = charCode;
        		AddGlyphs(charSet, fontSizeInAtlas, isBold, isItalic);
        	}

        	return glyphsByFontSize[charCode];
	    }
    }

    u64 FFontAtlas::ComputeMemoryFootprint()
    {
	    u64 size = Super::ComputeMemoryFootprint();

        for (const auto& atlasImageMip : atlasImageMips)
        {
            size += atlasImageMip->atlasSize * atlasImageMip->atlasSize;
            size += sizeof(FAtlasImage);
        }

        size += glyphDataList.GetCapacity() * sizeof(FGlyphData);
        size += glyphBuffer.GetElementCount() * sizeof(FGlyphData) * RHI::Limits::MaxSwapChainImageCount;

        size += mipIndicesByCharacter.GetSize() * sizeof(Pair<CharCode, FontSize>);

        return size;
    }

    void FFontAtlas::Flush(u32 imageIndex)
    {
        ZoneScoped;

        if (!flushRequiredPerImage[imageIndex])
            return;

        if (atlasUpdateRequired)
        {
            atlasUpdateRequired = false;

            delete atlasTexture; atlasTexture = nullptr;

            Array<CMImage> images;
            images.Reserve(atlasImageMips.GetSize());

            for (FAtlasImage* atlasImageMip : atlasImageMips)
            {
                CMImage image = CMImage::LoadRawImageFromMemory(atlasImageMip->ptr, atlasImageMip->atlasSize, atlasImageMip->atlasSize,
                    CMImageFormat::R8, CMImageSourceFormat::None, 8, 8);
                images.Add(image);
            }

            RHI::SamplerDescriptor fontSampler{};
            fontSampler.addressModeU = fontSampler.addressModeV = fontSampler.addressModeW = RHI::SamplerAddressMode::ClampToBorder;
            fontSampler.borderColor = RHI::SamplerBorderColor::FloatTransparentBlack;
            fontSampler.enableAnisotropy = false;
            fontSampler.samplerFilterMode = RHI::FilterMode::Cubic;

            atlasTexture = new RPI::Texture(images, fontSampler);

            fontSrg2->Bind("_FontAtlas", atlasTexture->GetRhiTexture());
            fontSrg2->Bind("_FontAtlasSampler", atlasTexture->GetSamplerState());
        }

        flushRequiredPerImage[imageIndex] = false;

        int numGlyphs = glyphDataList.GetCount();

        if (glyphBuffer.GetElementCount() < numGlyphs)
        {
            u64 totalCount = (u64)((f32)glyphBuffer.GetElementCount() * (1.0f + GlyphBufferGrowRatio));
            glyphBuffer.GrowToFit(Math::Max((u64)(numGlyphs)+64, totalCount));
        }

        glyphBuffer.GetBuffer(imageIndex)->UploadData(glyphDataList.GetData(), numGlyphs * sizeof(FGlyphData));

        fontSrg2->FlushBindings();
    }

    void FFontAtlas::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        glyphBuffer.Shutdown();

        delete atlasTexture; atlasTexture = nullptr;
        delete fontSrg2; fontSrg2 = nullptr;

        atlasImageMips.Clear();
        glyphDataList.Free();
    }

    void FFontAtlas::AddGlyphs(const Array<u32>& charSet, u32 fontSize, bool isBold, bool isItalic)
    {
        ZoneScoped;

        if (charSet.IsEmpty())
            return;

        FT_Face face = regular;

        if (isBold && isItalic)
        {
            if (boldItalic != nullptr)
                face = boldItalic;
            else if (bold != nullptr)
                face = bold;
        }
        else if (isBold)
        {
            if (bold != nullptr)
                face = bold;
        }
        else if (isItalic)
        {
            if (italic != nullptr)
                face = italic;
        }

        static HashSet<FT_ULong> nonDisplayCharacters = { ' ', '\n', '\r', '\t' };

        u32 dpi = PlatformApplication::Get()->GetSystemDpi();
        f32 scaling = PlatformApplication::Get()->GetSystemDpiScaling();

        FT_Set_Char_Size(face, 0, fontSize << 6, dpi, dpi);

        FAtlasImage* atlasMip = atlasImageMips[currentMip];

        for (int i = 0; i < charSet.GetSize(); ++i)
        {
            FT_ULong charCode = charSet[i];
            char c = charCode;

            if (atlasMip->glyphsByFontSize[fontSize].KeyExists(charCode))
            {
                continue;
            }

            FT_Error error = FT_Load_Char(face, charCode, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL);
            if (error != 0)
            {
	            continue;
            }

            FT_Bitmap* bmp = &face->glyph->bitmap;

            FFontGlyphInfo glyph{};
            
            glyph.charCode = charCode;

            int width = bmp->width;
            int height = bmp->rows;

            int xOffset = face->glyph->bitmap_left;
            int yOffset = face->glyph->bitmap_top;
            int advance = face->glyph->advance.x >> 6;

            int posX, posY;
            int atlasSize = atlasMip->atlasSize;

            bool foundEmptySpot = atlasMip->FindInsertionPoint(Vec2i(width + 1, height + 1), posX, posY);
            if (!foundEmptySpot)
            {
                currentMip++;
                atlasSize = FontAtlasSize / (SIZE_T)Math::Pow(2, currentMip);

                atlasMip = new FAtlasImage;
                atlasMip->ptr = new u8[atlasSize * atlasSize];
                memset(atlasMip->ptr, 0, atlasSize * atlasSize);
                atlasMip->atlasSize = (u32)atlasSize;

                atlasImageMips.Add(atlasMip);

                Array<String> pages = GetPages();
                pages.Add(String::Format("Page {}", pages.GetSize()));
                SetPages(pages);
                
                foundEmptySpot = atlasMip->FindInsertionPoint(Vec2i(width + 1, height + 1), posX, posY);
            }

            if (!foundEmptySpot)
            {
	            return;
            }

            if (foundEmptySpot)
            {
                glyph.atlasSize = atlasSize;

                glyph.x0 = posX;
                glyph.y0 = posY;
                glyph.x1 = posX + width;
                glyph.y1 = posY + height;

                glyph.xOffset = xOffset;
                glyph.yOffset = yOffset;
                glyph.advance = advance;
                glyph.fontSize = fontSize;
                glyph.index = glyphDataList.GetCount();

                glyph.scalingFactor = 1.0f;

                Vec2 uvMin = Vec2((f32)glyph.x0 / (f32)atlasSize, (f32)glyph.y0 / (f32)atlasSize);
                Vec2 uvMax = Vec2((f32)glyph.x1 / (f32)atlasSize, (f32)glyph.y1 / (f32)atlasSize);

                if (!nonDisplayCharacters.Exists(charCode))
                {
                    glyphDataList.Insert({ .atlasUV = Vec4(uvMin, uvMax), .mipIndex = (u32)currentMip });

                    for (int row = 0; row < bmp->rows; ++row)
                    {
                        for (int col = 0; col < bmp->width; ++col)
                        {
                            int x = posX + col;
                            int y = posY + row;
                            atlasMip->ptr[y * atlasSize + x] = bmp->buffer[row * bmp->pitch + col];
                        }
                    }

                    atlasUpdateRequired = true;
                }
                else
                {
                    glyphDataList.Insert({ .atlasUV = Vec4(-1, -1, -1, -1), .mipIndex = (u32)currentMip });
                }

                atlasMip->glyphsByFontSize[fontSize][charCode] = glyph;
                mipIndicesByCharacter[{ static_cast<unsigned int>(charCode), fontSize }] = currentMip;
                
                for (int j = 0; j < flushRequiredPerImage.GetSize(); ++j)
                {
                    flushRequiredPerImage[j] = true;
                }
            }
        }
    }

    bool FFontAtlas::FAtlasImage::FindInsertionPoint(Vec2i glyphSize, int& outX, int& outY)
    {
        ZoneScoped;

        int bestRowIndex = -1;
        int bestRowHeight = INT_MAX;

        if (rows.IsEmpty())
        {
            rows.Add({ .x = glyphSize.width, .y = 0, .height = glyphSize.height });
            outX = 0;
            outY = 0;
            return true;
        }

        for (int i = 0; i < rows.GetSize(); ++i) 
        {
            int x = rows[i].x;
            int y = rows[i].y;

            // Check if the glyph fits at this position
            if (x + glyphSize.width <= atlasSize && y + glyphSize.height <= atlasSize)
            {
                if (rows[i].height >= glyphSize.height && rows[i].height < bestRowHeight)
                {
                    bestRowHeight = rows[i].height;
                    bestRowIndex = i;
                }
            }
        }

        if (bestRowIndex == -1)
        {
            RowSegment lastRow = rows.Top();
            if (lastRow.y + lastRow.height + glyphSize.height > atlasSize)
            {
	            return false;
            }

            rows.Add({ .x = glyphSize.width, .y = lastRow.y + lastRow.height, .height = glyphSize.height });

            outX = 0;
            outY = rows.Top().y;

	        return true;
        }

        outX = rows[bestRowIndex].x;
        outY = rows[bestRowIndex].y;

        rows[bestRowIndex].x += glyphSize.width;

        return true;
    }

} // namespace CE
