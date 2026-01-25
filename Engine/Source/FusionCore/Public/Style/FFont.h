#pragma once

namespace CE
{
    ENUM()
    enum class FWordWrap : u8
    {
        Normal = 0,
        BreakWord,
        NoWrap
    };
    ENUM_CLASS_FLAGS(FWordWrap);

    STRUCT()
    struct FUSIONCORE_API FFont
    {
        CE_STRUCT(FFont)
    public:

        virtual ~FFont() = default;

        FFont()
        {}

        FFont(const Name& family, f32 fontSize = 10, b8 isBold = false, b8 isItalic = false)
    		: family(family), fontSize(fontSize), isBold(isBold), isItalic(isItalic)
        {}

        const Name& GetFamily() const { return family; }

        void SetFamily(const Name& family) { this->family = family; }

        f32 GetFontSize() const { return fontSize; }

        void SetFontSize(f32 fontSize) { this->fontSize = fontSize; }

        const bool& IsItalic() const { return isItalic; }

        const bool& IsBold() const { return isBold; }

        void SetItalic(bool set) { isItalic = set; }

    	void SetBold(bool set) { isBold = set; }

        bool operator==(const FFont& rhs) const
        {
            return family == rhs.family && fontSize == rhs.fontSize && isBold == rhs.isBold && isItalic == rhs.isItalic;
        }

        bool operator!=(const FFont& rhs) const
        {
	        return !operator==(rhs);
        }

        SIZE_T GetHash() const
        {
            SIZE_T hash = CE::GetHash(family);
            CombineHash(hash, (SIZE_T)fontSize);
            CombineHash(hash, (SIZE_T)isBold);
            CombineHash(hash, (SIZE_T)isItalic);
            return hash;
		}

    private:

        FIELD()
        Name family = "";

        FIELD()
        f32 fontSize = 10;

        FIELD()
        b8 isBold = false;

        FIELD()
        b8 isItalic = false;

    };
    
} // namespace CE

#include "FFont.rtti.h"