#pragma once

#include "Math/Math.h"

#include "Types/CoreTypeDefs.h"
#include "Misc/CoreDefines.h"

#include "PAL/Common/PlatformMemory.h"

#include <memory>

namespace CE
{
    class SharedMutex;
    class SharedRecursiveMutex;

    template<SIZE_T Length, SIZE_T Alignment = alignof(max_align_t)>
    using AlignedStorage = std::aligned_storage_t<Length, Alignment>;

    class CORE_API Memory
    {
    public:
        CE_STATIC_CLASS(Memory);
        
        static inline void* SystemMalloc(SIZE_T size)
        {
            return ::malloc(size);
        }

        static inline void SystemFree(void* pointer)
        {
            ::free(pointer);
        }

        static inline void* Malloc(SIZE_T size)
        {
            void* block = SystemMalloc(size);
            Zero(block, size);
            return block;
        }

        static inline void Free(void* pointer)
        {
            SystemFree(pointer);
        }

        static inline void Zero(void* block, SIZE_T size)
        {
            memset(block, 0, size);
        }

        /* Allocates memory with the Size aligned to specified Alignment. IMPORTANT: Alignment should always be a power of 2 */
        static inline void* Malloc(SIZE_T size, SIZE_T alignment)
        {
            auto AlignedSize = GetAlignedSize(size, alignment);
            return Malloc(AlignedSize);
        }

        /* Allocates memory with base address & size aligned to specified Alignment. IMPORTANT: Alignment should always be a power of 2 */
        static inline void* AlignedAlloc(SIZE_T size, SIZE_T alignment)
        {
            return PlatformMemory::AlignedAlloc(size, alignment);
        }

        static void* AlignedRealloc(void* block, SIZE_T size, SIZE_T alignment)
        {
            return PlatformMemory::AlignedRealloc(block, size, alignment);
        }

        static inline void AlignedFree(void* block)
        {
            PlatformMemory::AlignedFree(block);
        }

        static SIZE_T AlignedBlockSize(void* block, SIZE_T alignment, SIZE_T offset = 0)
        {
            return PlatformMemory::GetAlignedBlockSize(block, alignment, offset);
        }

        /* Returns a Size that is Aligned to the given Alignment. IMPORTANT: Alignment should always be a power of 2 */
        static inline SIZE_T GetAlignedSize(SIZE_T baseSize, SIZE_T alignment)
        {
            return (baseSize + alignment - 1) & ~(alignment - 1);
        }

        static inline SIZE_T AlignUp(SIZE_T baseSize, SIZE_T alignment)
        {
            assert(Math::IsPowerOf2(alignment));
            return (baseSize + alignment - 1) & ~(alignment - 1);
        }

        static std::atomic<SIZE_T> totalAllocation;

    };
}
