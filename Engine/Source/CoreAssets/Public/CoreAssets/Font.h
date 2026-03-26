#pragma once

namespace CE
{
    struct CMGlyphInfo 
    {
        u32 charCode = 0;
        int x0, y0, x1, y1;	// coords of glyph in the texture atlas
        int xOffset, yOffset;   // left & top bearing when rendering
        int advance;        // x advance when rendering
        int width; int height;
        int fontSize;
    };

    struct CMFontMetrics
    {
        f32 ascender = 0;
        f32 descender = 0;
        f32 lineGap = 0;
        f32 lineHeight = 0;
    };

    struct CharRange
    {
        CharRange() = default;
        CharRange(u32 c) { charCode = c; this->range = Vec2i(0, 0); }
        CharRange(s32 min, s32 max) { this->range = Vec2i(min, max); charCode = 0; }

        u32 charCode = 0;
        Vec2i range{};
    };

    struct CMFontAtlasGenerateInfo
    {
        Array<CharRange> charSetRanges{};
        u32 padding = 1;
        u32 fontSize = 16;
        int startOffsetX = 0;
        int startOffsetY = 0;
    };
    
    class COREASSETS_API CMFontAtlas final
    {
    public:

        ~CMFontAtlas();

        static CMFontAtlas* GenerateFromFontFile(const IO::Path& filePath, const CMFontAtlasGenerateInfo& generateInfo);

        inline const CMImage& GetAtlas() const { return atlas; }

        inline const Array<CMGlyphInfo>& GetGlyphInfos() const { return glyphInfos; }

        inline const CMFontMetrics& GetMetrics() const { return metrics; }

    private:

        CMImage atlas{};

        Array<CMGlyphInfo> glyphInfos{};

        CMFontMetrics metrics{};
    };

} // namespace CE
