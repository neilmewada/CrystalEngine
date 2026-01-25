#pragma once

namespace CE
{
	enum class CMImageFormat
	{
		Undefined = 0,
		// Grayscale
		R8,
		// Gray + Alpha
		RG8,
		RGB8,
		RGBA8,

		R32,
		RG32,
		RGB32,
		RGBA32,

		R16,
		RG16,
		RGB16,
		RGBA16,
		RGB565,

		BC1,
		BC3,
		BC4,
		BC5,
		BC6H,
		BC7
	};
	ENUM_CLASS_FLAGS(CMImageFormat);

	enum class CMImageSourceFormat
	{
		None = 0,
		PNG, JPG,
		HDR, EXR,
		BC1,
		BC3,
		BC4,
		BC5,
		BC6H,
		BC7,
	};

    struct CMImageInfo
    {
    public:
		union {
			u32 x = 0;
			u32 width;
		};
		union {
			u32 y = 0;
			u32 height;
		};
		u32 numChannels = 0;
		CMImageFormat format = CMImageFormat::Undefined;
		CMImageSourceFormat sourceFormat = CMImageSourceFormat::None;
		u32 bitDepth = 0;
		u32 bitsPerPixel = 0;

        const char* failureReason = nullptr;

        virtual bool IsValid() const { return x > 0 && y > 0 && bitDepth > 0 && bitsPerPixel > 0 && numChannels > 0 && numChannels <= 4 && 
			format != CMImageFormat::Undefined; }
    };

	/*
	* CMImage is a low-level image handling library.
	* It abstracts away all the image loading mechanisms to provide a 'raw' image to be used across the engine.
	* You have to manually call Free() function to destroy the image!
	*/
    class COREMEDIA_API CMImage : private CMImageInfo
    {
    public:
        CMImage();
        virtual ~CMImage();

        // - Static API -

        static CMImageInfo GetImageInfoFromFile(const IO::Path& filePath);
        static CMImageInfo GetImageInfoFromMemory(unsigned char* buffer, u32 bufferLength);

        static CMImage LoadFromFile(IO::Path filePath);

        static CMImage LoadFromMemory(unsigned char* buffer, u32 bufferLength);

		/// Loads raw image from memory without allocating any memory
		static CMImage LoadRawImageFromMemory(unsigned char* buffer, u32 width, u32 height, CMImageFormat pixelFormat, CMImageSourceFormat sourceFormat, u32 bitDepth, u32 bitsPerPixel);

		// - Encode API -

		// Encodes raw image pixel data to PNG format
		static bool EncodePNG(const CMImage& source, Stream* outStream, CMImageFormat pixelFormat, u32 bitDepth = 8);

		bool EncodeToPNG(const IO::Path& path) const;

        // - Public API -

        /// Always call the Free() function when image is no longer needed!
        void Free();

        inline void* GetDataPtr() const { return data; }
		inline u32 GetDataSize() const { return bitsPerPixel / 8 * x * y; }
        inline u32 GetWidth() const { return x; }
        inline u32 GetHeight() const { return y; }
        inline u32 GetNumChannels() const { return numChannels; }
		inline CMImageFormat GetFormat() const { return format; }
		inline CMImageSourceFormat GetSourceFormat() const { return sourceFormat; }
		inline u32 GetBitDepth() const { return bitDepth; }
		inline u32 GetBitsPerPixel() const { return bitsPerPixel; }

        bool IsValid() const override { return data != nullptr && CMImageInfo::IsValid(); }

        inline const char* GetFailureReason() const { return failureReason; }

    private:

		static CMImageInfo GetPNGImageInfo(MemoryStream* stream);
		static CMImage LoadPNGImage(MemoryStream* stream);

		static CMImageInfo GetJPGImageInfo(MemoryStream* stream);
		static CMImage LoadJPGImage(MemoryStream* stream);

        void* data = nullptr;
		bool allocated = true;

		friend class CMImageEncoder;
		friend class CMFontAtlas;
    };

}
