#pragma once

#include "Types/CoreTypeDefs.h"

#include <functional>

#define DECL_HASH(Type)\
template<>\
inline SIZE_T GetHash<Type>(const Type& value)\
{\
	return std::hash<Type>()(value);\
}

namespace CE
{
	template<typename T>
	class Array;

	struct Hash128
	{
		u64 high64;
		u64 low64;
	};

	CORE_API Hash128 CalculateHash128(const void* data, SIZE_T length);

	CORE_API SIZE_T CalculateHash(const void* data, SIZE_T length);

    /// Default implementation does not have any 'special' code other than for pointers. Specializations do all the work.
    template<typename T>
    SIZE_T GetHash(const T& value)
    {
        constexpr bool isPointer = std::is_pointer_v<T>;
        
        typedef std::remove_pointer_t<T> WithoutPtrType;
		typedef THasGetHashFunction<T> GetHashFuncType;
        
        if constexpr (isPointer)
        {
            return (SIZE_T)(WithoutPtrType*)value;
        }
		else if constexpr (GetHashFuncType::Value)
		{
			return GetHashFuncType::GetHash(&value);
		}
        else
        {
			return std::hash<T>()(value);
        }
    }

	template <class T>
	CE_INLINE void CombineHash(SIZE_T& seed, const T& v)
	{
		seed ^= GetHash<T>(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	CE_INLINE SIZE_T GetCombinedHash(SIZE_T hashA, SIZE_T hashB)
	{
		hashA ^= (hashB + 0x9e3779b9 + (hashA << 6) + (hashA >> 2));
		return hashA;
	}

	CORE_API u32 CalculateCRC(const void* data, SIZE_T size);
	CORE_API u32 CalculateCRC(const void* data, SIZE_T size, u32 crc);
	
	CORE_API SIZE_T GetCombinedHashes(const CE::Array<SIZE_T>& hashes);

	template<typename... Args>
	inline SIZE_T GetHashes(const Args&... values)
	{
		return GetCombinedHashes({ CE::GetHash<Args>(values)... });
	}

	DECL_HASH(u8)
	DECL_HASH(u16)
	DECL_HASH(u32)
	DECL_HASH(u64)

	DECL_HASH(s8)
	DECL_HASH(s16)
	DECL_HASH(s32)
	DECL_HASH(s64)

	DECL_HASH(f32)
	DECL_HASH(f64)

	DECL_HASH(bool)
	DECL_HASH(char)

	template<typename T>
	struct HashFunc
	{
		SIZE_T operator()(const T& value) const
		{
			return CE::GetHash(value);
		}
	};


} // namespace CE

