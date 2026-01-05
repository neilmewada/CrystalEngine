#pragma once

namespace CE::RHI
{
	class CommandList;
    class SwapChain;

	enum class HardwareQueueClass
	{
		Graphics = 0,

		Compute,

		//! Copy operations.
		Transfer,

		COUNT
	};
	ENUM_CLASS(HardwareQueueClass);

	constexpr u32 HardwareQueueClassCount = static_cast<u32>(HardwareQueueClass::COUNT);

	enum class HardwareQueueClassMask
	{
		None = 0,
		Graphics = BIT(static_cast<u32>(HardwareQueueClass::Graphics)),
		Compute = BIT(static_cast<u32>(HardwareQueueClass::Compute)),
		Transfer = BIT(static_cast<u32>(HardwareQueueClass::Transfer)),
		All = Graphics | Compute | Transfer
	};
	ENUM_CLASS_FLAGS(HardwareQueueClassMask);

    struct CommandQueueSubmission
    {
        u32 numCommandLists = 0;
        RHI::CommandList** commandLists = nullptr;
        
        RHI::Fence* waitFence = nullptr;
        uint64_t waitFenceValue = 0;

		//! @brief The pipeline stage to wait on for the wait fence. If Undefined, wait on all stages. Only used in Vulkan.
		RHI::ResourceState waitFenceStage = RHI::ResourceState::Undefined;
        
        RHI::Fence* signalFence = nullptr;
        uint64_t signalFenceValue = 0;
        
        u32 numPresentSwapChains = 0;
        RHI::SwapChain** presentSwapChains = nullptr;
    };

	class CORERHI_API CommandQueue : RHIResource
	{
	protected:
		CommandQueue() : RHIResource(ResourceType::CommandQueue)
		{}

		virtual ~CommandQueue() = default;

	public:
        
        virtual bool Submit(const CommandQueueSubmission& submission) = 0;

		HardwareQueueClassMask GetQueueMask() const
		{
			return queueMask;
		}
        
        inline HardwareQueueClass GetQueueClass() const
        {
            return queueClass;
        }

		HardwareQueueClass GetQueueClass() const
		{
			return queueClass;
		}

		bool SupportsOperation(HardwareQueueClass operationType) const
		{
			return (queueMask & (1 << (u32)operationType)) != 0;
		}

	protected:

		HardwareQueueClass queueClass{};

		HardwareQueueClassMask queueMask{};
        HardwareQueueClass queueClass{};

	};
    
} // namespace CE::RHI
