#include "CoreMinimal.h"

namespace CE
{
    OSAllocator* OSAllocator::Get()
    {
		static OSAllocator instance{};
        return &instance;
    }

    void* OSAllocator::allocate(size_t byteSize, size_t alignment)
    {
		return PlatformMemory::AlignedAlloc(byteSize, alignment);
    }

    void OSAllocator::deallocate(void* pointer, [[maybe_unused]] size_t byteSize, [[maybe_unused]] size_t alignment)
    {
		PlatformMemory::AlignedFree(pointer);
    }

    void* OSAllocator::reallocate(void* ptr, SizeType newSize, AlignType newAlignment)
    {
        return PlatformMemory::AlignedRealloc(ptr, newSize, newAlignment);
    }

} // namespace CE
