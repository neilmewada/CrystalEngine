#include "CoreMinimal.h"

namespace CE
{
	

	BinaryBlob::BinaryBlob()
	{
		
	}

	BinaryBlob::BinaryBlob(const BinaryBlob& copy)
	{
		Free();

		if (copy.IsValid())
		{
			LoadData(copy.data, copy.dataSize);
		}
	}

	BinaryBlob& BinaryBlob::operator=(const BinaryBlob& copy)
	{
        if (this == &copy)
            return *this;
        
		Free();

		if (copy.IsValid())
		{
			LoadData(copy.data, copy.dataSize);
		}

		return *this;
	}

	BinaryBlob::~BinaryBlob()
	{
		Free();
	}

	BinaryBlob::BinaryBlob(BinaryBlob&& move) noexcept
	{
		flags = move.flags;
		data = move.data;
		dataSize = move.dataSize;

		move.flags = BLOB_None;
		move.data = nullptr;
		move.dataSize = 0;
	}

    BinaryBlob& BinaryBlob::operator=(BinaryBlob&& move) noexcept
    {
        if (this == &move)
            return *this;
        
        flags = move.flags;
        data = move.data;
        dataSize = move.dataSize;

        move.flags = BLOB_None;
        move.data = nullptr;
        move.dataSize = 0;
        
        return *this;
    }

	void BinaryBlob::Free()
	{
		if (data != nullptr)
		{
			Memory::Free(data);
		}

		data = nullptr;
		dataSize = 0;
	}

	void BinaryBlob::Reserve(u64 byteSize)
	{
		u8* copy = (u8*)Memory::Malloc(byteSize);
		memset(copy, 0, byteSize);
        if (data != nullptr && dataSize > 0)
            memcpy(copy, data, dataSize);

        Free();

		data = copy;
		dataSize = byteSize;
	}

	bool BinaryBlob::IsValid() const
	{
		return data != nullptr && dataSize > 0;
	}

    void BinaryBlob::LoadData(const void* data, u64 dataSize)
    {
		if (dataSize <= 0)
		{
			Free();
			return;
		}

        Reserve(dataSize);
        memcpy(this->data, data, dataSize);
    }

	void BinaryBlob::LoadData(Stream* fromStream)
	{
		if (!fromStream->CanRead() || !fromStream->IsOpen())
			return;

		fromStream->SetBinaryMode(true);

		u64 length = fromStream->GetLength();
		Free();

		if (length > 0)
		{
			Reserve(length);
			fromStream->Read(data, dataSize);
		}
	}

	bool BinaryBlob::Deserialize(Stream* stream)
	{
		if (stream == nullptr || !stream->IsOpen() || !stream->CanRead())
			return false;
        
        s64 size = 0;
		*stream >> size;
		u64 flagsValue = 0;
		*stream >> flagsValue;

        if (size <= 0)
        {
			this->flags = (BinaryBlobFlags)flagsValue;
            Free();
            return true;
        }
		
		this->flags = (BinaryBlobFlags)flagsValue;
        
        Reserve((u64)size);
        
        stream->Read(data, (u64)size);
        
        return true;
	}

    bool BinaryBlob::Serialize(Stream* stream) const
    {
        if (stream == nullptr || !stream->IsOpen() || !stream->CanWrite())
			return false;

        *stream << (s64)dataSize;
		*stream << (u64)flags;

		if (dataSize > 0)
			stream->Write(data, dataSize);
		
        return true;
    }

} // namespace CE

CE_RTTI_POD_IMPL(CE, BinaryBlob)
