#pragma once

namespace CE::Vulkan
{

	class VULKANRHI_API CommandBufferAllocator final
	{
	public:
		CommandBufferAllocator(Device* device);
		virtual ~CommandBufferAllocator();

		VkCommandPool Allocate(u32 count, VkCommandBuffer* outBuffers, VkCommandBufferLevel level, u32 queueFamilyIndex);

		void Free(VkCommandPool pool, u32 count, VkCommandBuffer* buffers);

	private:

		struct Context
		{
			CommandBufferAllocator* allocator = nullptr;
			Device* device = nullptr;
		};

		struct CommandPool
		{
			VkCommandPool handle = nullptr;
			u32 queueFamilyIndex = 0;
		};

		struct Allocator
		{
			Allocator();
			virtual ~Allocator();

			VkCommandPool GetOrCreate(u32 queueFamilyIndex);

			bool valid = false;
			Context context{};
			List<CommandPool> commandPools{};
		};

		Device* device = nullptr;

		ThreadLocalContext<Allocator> threadedAllocator;

		mutable SharedMutex mutex{};

		static Array<Context> commandAllocatorContexts;
	};
    
} // namespace CE::Vulkan
