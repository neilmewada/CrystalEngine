#pragma once

namespace CE
{

	struct VirtualAddress
	{
		constexpr VirtualAddress() : ptr(static_cast<uintptr_t>(-1))
		{}

		constexpr VirtualAddress(uintptr_t value) : ptr(value)
		{}

		constexpr operator uintptr_t() const
		{
			return ptr;
		}

		constexpr bool operator==(const VirtualAddress& rhs) const
		{
			return ptr == rhs.ptr;
		}

		constexpr bool operator!=(const VirtualAddress& rhs) const
		{
			return !operator==(rhs);
		}

		constexpr bool IsValid() const
		{
			return ptr != static_cast<uintptr_t>(-1);
		}

		uintptr_t ptr;
	};
    
	class CORE_API IAllocator
	{
	public:
		using SizeType = SIZE_T;
		using AlignType = SIZE_T;

		IAllocator() = default;
		virtual ~IAllocator() = default;
		CE_NO_COPY_MOVE(IAllocator);

		virtual void* allocate(SizeType byteSize, AlignType alignment = 1) = 0;
		virtual void deallocate(void* pointer, SizeType byteSize = 0, AlignType alignment = 0) = 0;
		virtual void* reallocate(void* ptr, SizeType newSize, AlignType newAlignment = 1) = 0;

		bool operator==(const IAllocator& rhs) const
		{
			return this == &rhs;
		}

	};


} // namespace CE
