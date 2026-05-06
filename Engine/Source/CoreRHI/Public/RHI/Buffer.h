#pragma once

namespace CE::RHI
{
	struct BufferDescriptor
	{
		/// @brief Name used for debugging purposes.
		Name name = "Buffer";
		
		u64 bufferSize = 0;

		u64 structureByteStride = 0;

		u64 alignment = 0;
		
		BufferBindFlags bindFlags{};

		MemoryHeapType defaultHeapType{};

		SIZE_T GetHash() const
		{
			SIZE_T hash = CE::GetHash(bufferSize);
			CombineHash(hash, bindFlags);
			CombineHash(hash, structureByteStride);
			return hash;
		}

		bool operator==(const BufferDescriptor& rhs) const
		{
			return bufferSize == rhs.bufferSize && bindFlags == rhs.bindFlags && structureByteStride == rhs.structureByteStride;
		}
	};

	class MemoryHeap;

	struct ResourceMemoryRequirements
	{
		SIZE_T size = 0;
		SIZE_T offsetAlignment = 0;
        /// @brief For internal use only!
		u32 flags = 0;
	};

	struct ResourceMemoryDescriptor
	{
		RHI::MemoryHeap* memoryHeap = nullptr;
		u64 memoryOffset = 0;
	};

	class CORERHI_API Buffer : public RHIResource, public IDeviceObject
	{
	protected:
		Buffer() : RHIResource(ResourceType::Buffer), IDeviceObject(DeviceObjectType::Buffer)
		{}

	public:
		virtual ~Buffer() = default;

		inline BufferBindFlags GetBindFlags() const
		{
			return bindFlags;
		}

		inline u64 GetStructureByteStride() const
		{
			return structureByteStride;
		}

		virtual void* GetHandle() = 0;

		inline u64 GetBufferSize() const
		{
			return bufferSize;
		}

		//! Returns true if buffer data is directly accessibly on Host (CPU) by mapping & unmapping memory
		virtual bool IsHostAccessible() const = 0;

		virtual void UploadData(const BufferData& data) = 0;

		inline void UploadData(const void* data, u64 dataSize, u64 startOffsetInBuffer = 0)
		{
			RHI::BufferData bufferData{};
			bufferData.data = data;
			bufferData.dataSize = dataSize;
			bufferData.startOffsetInBuffer = startOffsetInBuffer;
			UploadData(bufferData);
		}

		virtual bool Map(u64 offset, u64 size, void** outPtr) = 0;
		virtual bool Unmap() = 0;

		//! Allocates a raw buffer in CPU memory and reads buffer data into it. You are responsible for releasing outData memory block using Memory::Free().
		virtual void ReadData(u8** outData, u64* outDataSize) = 0;

		//! Read data into a pre-allocated memory.
		virtual void ReadData(void* data) = 0;

	protected:

		/// @brief Name used for debugging purposes.
		Name name{};

		BufferBindFlags bindFlags{};

		u64 bufferSize = 0;

		u64 structureByteStride = 0;

		u64 alignment = 0;

		RHI::MemoryHeapType heapType = RHI::MemoryHeapType::Default;
	};
    
} // namespace CE::RHI
