#pragma once

namespace CE::Vulkan
{
	class Buffer;

	class MemoryHeap final : public RHI::MemoryHeap
	{
	public:
		MemoryHeap(Device* device, const RHI::MemoryHeapDescriptor& desc);

		~MemoryHeap();

		bool SupportsOptimalImageTiling();

		bool BindBuffer(Buffer* buffer, VkDeviceSize offset);

		inline VkDeviceMemory GetHandle() const
		{
			return allocation;
		}

	private:

		struct Range
		{
			u64 min = 0, max = 0;
		};

		Array<Range> allocatedRanges{};
		VkDeviceMemory allocation{};

		Device* device = nullptr;
		VkMemoryPropertyFlags memoryPropertyFlags{};
		u32 supportedMemoryTypeBitMask = 0;
		u32 allocatedMemoryTypeIndex = 0;

	};
    
} // namespace CE::Vulkan
