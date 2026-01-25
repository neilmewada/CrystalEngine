#pragma once

typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;

namespace CE
{
    //! @brief Font metrics calculated for a unit font size. Multiply each field with the actual font size
    struct FFontMetrics
    {
        f32 ascender = 0;
        f32 descender = 0;
        f32 lineGap = 0;
        f32 lineHeight = 0;
    };

    struct FFontGlyphInfo
    {
        u32 charCode = 0;
        int x0 = 0;
        int y0 = 0;
        int x1 = 0;
        int y1 = 0;
        int xOffset = 0; // left bearing when rendering
        int yOffset = 0;    // top bearing when rendering
        int advance = 0;        // x advance when rendering
        u32 fontSize = 0; // Font size used when baking this glyph
        u32 index = 0; // User specific data: Stores the index of the glyph into the character buffer

        u32 atlasSize = 0;
        f32 scalingFactor = 1;

        inline int GetWidth() const { return x1 - x0; }
        inline int GetHeight() const { return y1 - y0; }
    };

    CLASS()
    class FUSIONCORE_API FFontAtlas : public Object
    {
        CE_CLASS(FFontAtlas, Object)
    public:

        static constexpr const char* ImageNamePrefix = "__FontAtlas_";

        FFontAtlas();

        void Init(const Array<u32>& characterSet);

        //! @brief Font metrics calculated for a unit font size. Multiply each field with the actual font size
        const FFontMetrics& GetMetrics() const { return metrics; }

        u32 GetArrayLayerCount() const { return atlasImageMips.GetSize(); }

        // - Public API -

        u32 GetAtlasSize() const;

        RHI::ShaderResourceGroup* GetFontSrg2() const { return fontSrg2; }

        FFontGlyphInfo FindOrAddGlyph(u32 charCode, u32 fontSize, bool isBold, bool isItalic);

        u64 ComputeMemoryFootprint() override;

        //! @brief Flushes all the changes to GPU
        void Flush(u32 imageIndex);

        // - Model -

        MODEL_PROPERTY(Array<String>, Pages);

    private:

        void OnBeginDestroy() override;

        void AddGlyphs(const Array<u32>& characterSet, u32 fontSize, bool isBold = false, bool isItalic = false);

        RPI::Texture* fontAtlas = nullptr;

    private:

        using CharCode = u32;
        using FontSize = u32;

        struct RowSegment {
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

            HashMap<u32, HashMap<CharCode, FFontGlyphInfo>> glyphsByFontSize;

            bool TryInsertGlyphRect(Vec2i glyphSize, int& outX, int& outY);
        };

        struct alignas(16) FGlyphData
        {
            Vec4 atlasUV;
            u32 mipIndex = 0;
        };

        using FGlyphDataList = StableDynamicArray<FGlyphData, 256, false>;

        FGlyphDataList glyphDataList;
        RPI::DynamicStructuredBuffer<FGlyphData> glyphBuffer;

        HashMap<Pair<CharCode, FontSize>, int> mipIndicesByCharacter;

        Array<Ptr<FAtlasImage>> atlasImageMips;
        int currentMip = 0;
        StaticArray<bool, RHI::Limits::MaxSwapChainImageCount> flushRequiredPerImage;
        bool atlasUpdateRequired = false;

        RPI::Texture* atlasTexture = nullptr;
        RHI::ShaderResourceGroup* fontSrg2 = nullptr;

        FFontMetrics metrics{};

        FT_Library ft = nullptr;
        FT_Face regular = nullptr; u8* regularData = nullptr;
        FT_Face italic = nullptr; u8* italicData = nullptr;
        FT_Face bold = nullptr; u8* boldData = nullptr;
        FT_Face boldItalic = nullptr; u8* boldItalicData = nullptr;

        friend class FFontManager;
        friend class FusionRenderer;
    };
    
} // namespace CE

#include "FFontAtlas.rtti.h"