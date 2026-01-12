
#include "Algorithm/Hash.h"
#include "Containers/Array.h"

#include "xxhash.h"
#include "CRC.h"

namespace CE
{
	CORE_API Hash128 CalculateHash128(const void* data, SIZE_T length)
	{
		ZoneScoped;

		XXH128_hash_t hash = XXH3_128bits(data, length);
		Hash128 out;
		out.high64 = hash.high64;
		out.low64 = hash.low64;
		return out;
	}

	CORE_API SIZE_T CalculateHash(const void* data, SIZE_T length)
	{
		ZoneScoped;

#if IS_64BIT
		return XXH64(data, length, 0);
#else
		return XXH32(data, length, 0);
#endif
	}

	CORE_API u32 CalculateCRC(const void* data, SIZE_T size)
	{
		ZoneScoped;

		return CRC::Calculate(data, size, CRC::CRC_32());
	}

	CORE_API u32 CalculateCRC(const void* data, SIZE_T size, u32 crc)
	{
		ZoneScoped;

		return CRC::Calculate(data, size, CRC::CRC_32(), crc);
	}

	CORE_API SIZE_T GetCombinedHashes(const CE::Array<SIZE_T>& hashes)
	{
		ZoneScoped;

		if (hashes.GetSize() == 0)
			return 0;
		
		SIZE_T hash = hashes[0];

		for (int i = 1; i < hashes.GetSize(); i++)
		{
			hash = GetCombinedHash(hash, hashes[i]);
		}

		return hash;
	}

}
