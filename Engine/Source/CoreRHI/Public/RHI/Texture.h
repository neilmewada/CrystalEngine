#pragma once

namespace CE::RHI
{

	struct TextureDescriptor
	{
		Name name{};
		u32 width = 0, height = 0, depth = 1;
		Dimension dimension = Dimension::Dim2D;
		Format format{};
		u32 mipLevels = 1;
		u32 sampleCount = 1;
		u32 arrayLayers = 1;
		TextureBindFlags bindFlags = TextureBindFlags::ShaderRead;
		MemoryHeapType defaultHeapType = MemoryHeapType::Default;
	};

    using ImageDescriptor = TextureDescriptor;

	class TextureView;

	class CORERHI_API Texture : public RHIResource, public IDeviceObject
	{
	protected:
		Texture() : RHIResource(ResourceType::Texture), IDeviceObject(DeviceObjectType::Texture)
		{}

	public:
		virtual ~Texture() = default;

		virtual void* GetHandle() = 0;
        
        inline u32 GetWidth() const { return width; }
        
		inline u32 GetHeight() const { return height; }
        
		inline u32 GetDepth() const { return depth; }

		inline u32 GetWidth(u32 mipSlice) const { return Math::Max<u32>(width > 0 ? 1 : 0, width / (u32)pow(2, mipSlice)); }

		inline u32 GetHeight(u32 mipSlice) const { return Math::Max<u32>(height > 0 ? 1 : 0, height / (u32)pow(2, mipSlice)); }

		inline u32 GetDepth(u32 mipSlice) const { return Math::Max((u32)1, depth / (u32)pow(2, mipSlice)); }

		inline u32 GetArrayLayerCount() const { return arrayLayers; }

		inline u32 GetMipLevelCount() const { return mipLevels; }

		inline const Name& GetDebugName() const { return name; }

		inline RHI::Format GetFormat() const { return format; }

		inline Dimension GetDimension() const { return dimension; }

		inline u32 GetSampleCount() const { return sampleCount; }

		inline TextureBindFlags GetBindFlags() const { return bindFlags; }
        
		inline u64 GetByteSize() const { return byteSize; }
        
		virtual u32 GetNumberOfChannels() = 0;

		virtual u32 GetBitsPerPixel() = 0;

		static u64 CalculateTotalTextureSize(u32 width, u32 height, u32 bitsPerPixel, u32 arrayCount = 1, u32 mipLevelCount = 1)
		{
			u64 size = 0;

			for (int mip = 0; mip < mipLevelCount; mip++)
			{
				u32 power = (u32)pow(2, mip);
				u64 curSize = width / power * height / power * bitsPerPixel / 8 * arrayCount;
				size += curSize;
			}

			return size;
		}

	protected:

		Name name{};

		u64 byteSize = 0;
		u32 width = 0, height = 0, depth = 0;
		Dimension dimension = Dimension::Dim2D;
		Format format{};
		u32 mipLevels = 1;
		u32 sampleCount = 1;
		u32 arrayLayers = 1;
		TextureBindFlags bindFlags = TextureBindFlags::None;
		
	};
    
} // namespace CE::RHI
