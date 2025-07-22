#pragma once

namespace CE
{
    struct FSDFFontAtlasInitInfo
    {
		RPI::Shader* sdfGlyphShader = nullptr;
	    Name fontName;
        Array<u32> characterSet;
	};

    CLASS()
    class FUSIONCORE_API FSDFFontAtlas : public Object
    {
        CE_CLASS(FSDFFontAtlas, Object)
    protected:

        FSDFFontAtlas();
        
    public:

		static constexpr u32 SDFFontSize = 64; // Size of the SDF font atlas in pixels

        void Init(const FSDFFontAtlasInitInfo& initInfo);

        //! @brief Flushes all the changes to GPU
        void Flush(u32 imageIndex);

        FFontGlyphInfo FindOrAddGlyph(u32 charCode, u32 fontSize, bool isBold, bool isItalic);

        RHI::ShaderResourceGroup* GetFontSrg2() const { return fontSrg2; }

        u32 GetAtlasSize() const;

	private:

		void OnBeginDestroy() override;

        void AddGlyphs(const Array<u32>& characterSet, bool isBold = false, bool isItalic = false);

        using CharCode = u32;
        using FontSize = u32;

        struct RowSegment
    	{
            int x, y;
            int height;
        };

        struct FAtlasImage : IntrusiveBase
        {
            virtual ~FAtlasImage()
            {
                delete ptr; ptr = nullptr;
            }

            u8* ptr = nullptr;
            u32 atlasSize = 0;
            Array<RowSegment> rows;

            HashMap<CharCode, FFontGlyphInfo> glyphsByCharCode;

            bool TryInsertGlyphRect(Vec2i glyphSize, int padding, int& outX, int& outY);
        };

        struct alignas(16) FGlyphData
        {
            Vec4 atlasUV;
            u32 layerIndex = 0;
        };

        FIELD()
        int padding = 16;

        using FGlyphDataList = StableDynamicArray<FGlyphData, 256, false>;

        FGlyphDataList glyphDataList;
        RPI::DynamicStructuredBuffer<FGlyphData> glyphBuffer;

        Array<Ptr<FAtlasImage>> atlasImageLayers;
        int currentLayer = 0;
        bool tempSaved = false;

        HashMap<CharCode, int> arrayLayerByCharCode;

        RPI::Shader* sdfGlyphShader = nullptr;

        StaticArray<bool, RHI::Limits::MaxSwapChainImageCount> flushRequiredPerImage;
        bool atlasUpdateRequired = true;

        RPI::Texture* atlasTexture = nullptr;
        RHI::ShaderResourceGroup* fontSrg2 = nullptr;

		RHI::RenderTarget* sdfRenderTarget = nullptr;
        RPI::Material* sdfGenMaterial = nullptr;

        FFontMetrics metrics{};

        FT_Library ft = nullptr;
        FT_Face regular = nullptr; u8* regularData = nullptr;
        FT_Face italic = nullptr; u8* italicData = nullptr;
        FT_Face bold = nullptr; u8* boldData = nullptr;
        FT_Face boldItalic = nullptr; u8* boldItalicData = nullptr;

        friend class FFontManager;
        friend class FusionRenderer2;
    };
    
} // namespace CE

#include "FSDFFontAtlas.rtti.h"
