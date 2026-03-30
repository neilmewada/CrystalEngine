#pragma once

namespace CE
{
    struct CAGlyphInfo 
    {
        u32 charCode = 0;
        int x0, y0, x1, y1;	// coords of glyph in the texture atlas
        int xOffset, yOffset;   // left & top bearing when rendering
        int advance;        // x advance when rendering
        int width; int height;
        int fontSize;
    };

    struct CAFontMetrics
    {
        f32 ascender = 0;
        f32 descender = 0;
        f32 lineGap = 0;
        f32 lineHeight = 0;
    };

    struct CACharRange
    {
        CACharRange() = default;
        CACharRange(u32 c) { charCode = c; this->range = Vec2i(0, 0); }
        CACharRange(s32 min, s32 max) { this->range = Vec2i(min, max); charCode = 0; }

        u32 charCode = 0;
        Vec2i range{};
    };

    struct CAFontAtlasGenerateInfo
    {
        Array<CACharRange> charSetRanges{};
        u32 padding = 1;
        u32 fontSize = 16;
        int startOffsetX = 0;
        int startOffsetY = 0;
    };
    
    class COREASSETS_API CAFontAtlas final
    {
		CE_NO_COPY(CAFontAtlas);
    public:

        ~CAFontAtlas();

		CAFontAtlas(CAFontAtlas&& move) noexcept;

		CAFontAtlas& operator=(CAFontAtlas&& move) noexcept;

        static CAFontAtlas* GenerateFromFontFile(const IO::Path& filePath, const CAFontAtlasGenerateInfo& generateInfo);

        inline const CAImage& GetAtlas() const { return atlas; }

        inline const Array<CAGlyphInfo>& GetGlyphInfos() const { return glyphInfos; }

        inline const CAFontMetrics& GetMetrics() const { return metrics; }

    private:

		CAFontAtlas() = default;

        CAImage atlas{};

        Array<CAGlyphInfo> glyphInfos{};

        CAFontMetrics metrics{};
    };

} // namespace CE
