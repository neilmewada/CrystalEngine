#pragma once

namespace CE
{
	enum BinaryBlobFlags : u64
	{
		BLOB_None = 0
	};
	ENUM_CLASS_FLAGS(BinaryBlobFlags);

	

	/// Bulk data
	class CORE_API BinaryBlob
	{
	public:

		BinaryBlob();
		BinaryBlob(const BinaryBlob& copy);
		BinaryBlob& operator=(const BinaryBlob& copy);

		~BinaryBlob();

		BinaryBlob(BinaryBlob&& move) noexcept;
        BinaryBlob& operator=(BinaryBlob&& move) noexcept;

		void Free();
		void Reserve(u64 byteSize);

		bool IsValid() const;
        
        void LoadData(const void* data, u64 dataSize);

		/// Loads raw binary blob from stream
		void LoadData(Stream* fromStream);

		bool Deserialize(Stream* stream);
        bool Serialize(Stream* stream) const;

		inline u8* GetDataPtr() const { return data; }
		inline u64 GetDataSize() const { return dataSize; }

		inline BinaryBlobFlags GetFlags() const { return flags; }
		inline void SetFlags(BinaryBlobFlags value) { flags = value; }
		inline void SetFlagEnabled(BinaryBlobFlags flag, bool enabled)
		{
			if (enabled)
				EnumAddFlags(flags, flag);
			else
				EnumRemoveFlags(flags, flag);
		}

		friend inline Stream& operator<<(Stream& stream, const BinaryBlob& blob)
		{
			blob.Serialize(&stream);
			return stream;
		}

		friend inline Stream& operator>>(Stream& stream, BinaryBlob& blob)
		{
			blob.Deserialize(&stream);
			return stream;
		}

	private:

		b8 isAllocated = false;
		
		BinaryBlobFlags flags = BLOB_None;

		u8* data = nullptr;
		u64 dataSize = 0;
	};
    
} // namespace CE

CE_RTTI_POD(CORE_API, CE, BinaryBlob)
