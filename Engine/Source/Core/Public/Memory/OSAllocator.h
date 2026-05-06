#pragma once

namespace CE
{
    
	class CORE_API OSAllocator : public IAllocator
	{
	public:

		OSAllocator() = default;

		static OSAllocator* Get();

		void* allocate(size_t byteSize, size_t alignment = 1) override;
		void deallocate(void* pointer, size_t byteSize = 0, size_t alignment = 0) override;
		void* reallocate(void* ptr, SizeType newSize, AlignType newAlignment = 1) override;

		bool operator==(const OSAllocator& rhs) const
		{
			return this == &rhs;
		}

	private:


	};

} // namespace CE
