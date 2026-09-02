#pragma once

namespace CE::RHI
{
	struct MemoryTypeMask
	{
		u32 value = 0;

		bool IsCompatibleWith(const MemoryTypeMask& other) const
		{
			return (value & other.value) != 0;
		}
	};

	enum class MemoryHeapType
	{
		/// @brief Allocates memory that is only accessible to the GPU.
		Default = 0,
		/// @brief Allocates CPU visible memory that is used to send data from CPU to GPU.
		Upload,
		/// @brief Allocates CPU visible memory that is used to send data from GPU to CPU.
		ReadBack
	};

	enum class MemoryHeapUsageFlags
	{
		None = 0,
		/// @brief Memory heap will be used to store buffers
		Buffer = BIT(0),
		/// @brief Memory heap will be used to store textures
		Texture = BIT(1),

		All = Buffer | Texture
	};
	ENUM_CLASS_FLAGS(MemoryHeapUsageFlags);

	struct MemoryHeapDescriptor
	{
		Name debugName = "MemoryHeap";
		MemoryHeapType heapType{};
		u64 allocationSize = 0;
		MemoryHeapUsageFlags usageFlags = MemoryHeapUsageFlags::All;
		MemoryTypeMask compatibleMemoryTypes{};
	};

	class CORERHI_API MemoryHeap : public RHIResource
	{
	protected:
		MemoryHeap() : RHIResource(RHI::ResourceType::MemoryHeap)
		{}

	public:

		inline const Name& GetName() const
		{
			return debugName;
		}

		inline MemoryHeapType GetHeapType() const
		{
			return heapType;
		}

		inline u64 GetHeapSize() const
		{
			return heapSize;
		}

		inline MemoryHeapUsageFlags GetUsageFlags() const
		{
			return usageFlags;
		}

	protected:

		Name debugName = "MemoryHeap";

		u64 heapSize = 0;

		MemoryHeapType heapType = MemoryHeapType::Default;

		MemoryHeapUsageFlags usageFlags{};
	};
    
} // namespace CE::RHI
