#pragma once

namespace CE::Vulkan
{
	class FrameGraphCompiler;
	class Scope;
}

namespace CE::Metal
{
    class FrameGraphCompiler;
    class Scope;
}

namespace CE::RHI
{
	class FrameGraph;
	struct FrameGraphCompileRequest;

	enum class ScopeOperation
	{
		Rasterization = 0,
		Compute,
		RayTracing,
		Transfer,
		AccelerationStructureBuild
	};

	struct ScopeDescriptor
	{
		ScopeId id{};
		RHI::HardwareQueueClass queueClass{};
		ScopeOperation operation = ScopeOperation::Rasterization;
	};

	struct ScopeGroup
	{
		ScopeId groupId{};
		Array<Scope*> scopes{};
	};

    class CORERHI_API Scope
    {
    public:
        virtual ~Scope();
        
	protected:

		Scope(const ScopeDescriptor& desc);

	public:

		inline ScopeId GetId() const { return id; }

		void AddScopeAttachment(ScopeAttachment* attachment);

		bool ScopeAttachmentExists(const Name& id);

		ScopeAttachment* FindScopeAttachment(const Name& id);

		const Array<ScopeAttachment*>& GetAttachments() const { return attachments; }

		template<typename ScopeAttachmentType, typename DescriptorType = typename ScopeAttachmentType::DescriptorType> requires TIsBaseClassOf<ScopeAttachment, ScopeAttachmentType>::Value
		ScopeAttachmentType* EmplaceScopeAttachment(FrameAttachment* attachment,
			ScopeAttachmentUsage usage,
			ScopeAttachmentAccess access,
			const DescriptorType& descriptor);

		inline bool PresentsSwapChain() const { return presentSwapChains.NotEmpty(); }

		bool Compile(const FrameGraphCompileRequest& compileRequest);

		bool UsesAttachment(FrameAttachment* attachment);

		bool UsesAttachment(AttachmentID attachmentId);
        
        const auto& GetProducers() const { return producers; }
        const auto& GetConsumers() const { return consumers; }
        
        RHI::Scope* GetPrevSubPass() const { return prevSubPass; }
        RHI::Scope* GetNextSubPass() const { return nextSubPass; }

		//void SetPassSrgLayout(const RHI::ShaderResourceGroupLayout& layout) { passSrgLayout = layout; }

		static HashMap<ScopeAttachment*, ScopeAttachment*> FindCommonFrameAttachments(Scope* from, Scope* to);

		inline bool IsSubPass() const
		{
			return prevSubPass != nullptr || nextSubPass != nullptr;
		}

		RHI::HardwareQueueClass GetQueueClass() const { return queueClass; }

    	ScopeOperation GetOperation() const { return operation; }

    	bool IsGraphicsPass() const { return operation == ScopeOperation::Rasterization; }
    	bool IsComputePass() const { return operation == ScopeOperation::Compute; }
		bool IsTransferPass() const { return operation == ScopeOperation::Transfer; }

		void SetShaderResourceGroups(const Array<RHI::ShaderResourceGroup*>& srgs);
		void AddShaderResourceGroups(RHI::ShaderResourceGroup* srg);

		int GetTimelineLevel() const { return timelineLevel; }

	protected:

		virtual bool CompileInternal(const FrameGraphCompileRequest& compileRequest) { return false; }

		//! @brief The frame graph that owns this scope.
		FrameGraph* frameGraph = nullptr;

		RHI::HardwareQueueClass queueClass{};
    	ScopeOperation operation = ScopeOperation::Rasterization;

		Array<RHI::PipelineState*> usePipelines;

		u32 groupCountX = 1;
		u32 groupCountY = 1;
		u32 groupCountZ = 1;

		ScopeId id{};
		int scopeGroupIndex = -1;
		//bool usesSwapChainAttachment = false;
		Array<SwapChain*> swapChainsUsedByAttachments;

		Scope* prevSubPass = nullptr;
		Scope* nextSubPass = nullptr;

		int timelineLevel = 0;
		int remainingProducers = 0;

		Array<Scope*> producers;
		Array<Scope*> consumers;

		Scope* prev = nullptr;
		Scope* next = nullptr;

		DrawList* drawList = nullptr;

		RHI::ShaderResourceGroupLayout passSrgLayout;

		RHI::ShaderResourceGroup* passShaderResourceGroup = nullptr;
		RHI::ShaderResourceGroup* subpassShaderResourceGroup = nullptr;

		Array<RHI::ShaderResourceGroup*> externalShaderResourceGroups{};
        
		//! @brief List of all scope attachments owned by this scope.
		Array<ScopeAttachment*> attachments{};

		Array<ImageScopeAttachment*> imageAttachments{};
		Array<BufferScopeAttachment*> bufferAttachments{};
		Array<ScopeAttachment*> readAttachments{};
		Array<ScopeAttachment*> writeAttachments{};

		Array<RHI::SwapChain*> presentSwapChains{};

		friend class FrameGraph;
        friend class FrameGraphCompiler;
		friend class CE::Vulkan::FrameGraphCompiler;
		friend class CE::Vulkan::Scope;
        friend class CE::Metal::FrameGraphCompiler;
        friend class CE::Metal::Scope;
		friend class FrameGraphBuilder;
		friend class FrameScheduler;
    };

	template<typename ScopeAttachmentType, typename DescriptorType> requires TIsBaseClassOf<ScopeAttachment, ScopeAttachmentType>::Value
	inline ScopeAttachmentType* Scope::EmplaceScopeAttachment(FrameAttachment* attachment, 
		ScopeAttachmentUsage usage, 
		ScopeAttachmentAccess access, 
		const DescriptorType& descriptor)
	{
		ScopeAttachmentType* scopeAttachment = new ScopeAttachmentType(this, attachment, usage, access, descriptor);
		AddScopeAttachment(scopeAttachment);
		return scopeAttachment;
	}

} // namespace CE::RHI
