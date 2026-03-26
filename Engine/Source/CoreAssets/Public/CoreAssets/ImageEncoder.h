#pragma once

namespace CE
{
    
    class COREASSETS_API CMImageEncoder final
    {
    public:

        enum Quality
        {
            Quality_Preview = 0,
            Quality_Fast,
            Quality_Normal,
            Quality_Slow
        };

        CMImageEncoder();
        ~CMImageEncoder();

        inline const String& GetErrorMessage() const { return errorMessage; }

        u64 GetCompressedSizeRequirement(const CMImage& image, CMImageSourceFormat destBCnFormat);

        bool EncodeToBCn(const CMImage& image, void* outData, CMImageSourceFormat destBCnFormat, Quality quality = Quality_Normal);

    private:

        String errorMessage = "";

    };

} // namespace CE
