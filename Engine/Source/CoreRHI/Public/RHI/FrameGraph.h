#pragma once

class RHI_FrameGraphBuilder_Test;

namespace CE::Metal
{
    class FrameGraphCompiler;
    class FrameGraphExecuter;
}

namespace CE::Vulkan
{
	class FrameGraphCompiler;
	class FrameGraphExecuter;
}

namespace CE::RHI
{
    class CORERHI_API FrameGraph final
    {
    public:
        FrameGraph();
        virtual ~FrameGraph();

		inline u32 GetSwapChainCount() const { return presentSwapChains.GetSize(); }

		inline SwapChain* GetSwapChain(u32 index) const { return presentSwapChains[index]; }
        
        const auto& GetPresentSwapChains() const { return presentSwapChains; }
        
        const Array<Scope*>& GetProducers() const { return producers; }
        
        const Array<Scope*>& GetEndScopes() const { return endScopes; }

		void DeleteAll();
        
		bool Build();

		struct GraphNode
		{
			GraphNode(Scope* scope = nullptr) : scope(scope)
			{}

			Scope* scope = nullptr;
			Array<Scope*> producers{};
			Array<Scope*> consumers{};

			inline SIZE_T GetHash() const
			{
				return (SIZE_T)scope;
			}

			inline bool operator==(const GraphNode& rhs) const
			{
				return scope == rhs.scope;
			}
		};

		bool ScopeHasDependency(Scope* source, Scope* dependentOn);

		void AddScopeDependency(Scope* from, Scope* to);

		void FinalizeGraph();

		HashMap<AttachmentID, Scope*> lastWrittenAttachmentToScope{};
		HashMap<AttachmentID, HashSet<Scope*>> attachmentReadSchedule{};
		HashMap<Scope*, HashSet<Scope*>> nodeDependencies{};
		HashMap<ScopeId, GraphNode> nodes{};

        //! A database of all attachments used in this frame graph.
        FrameAttachmentDatabase attachmentDatabase{};

        Array<Scope*> scopes{};
		HashMap<ScopeId, Scope*> scopesById{};
		HashMap<int, Array<Scope*>> scopesByTimelineLevel{};
		int maxTimelineLevel = 0;

		//! Multiple scopes can be grouped together as subpasses in a single render pass.
		Array<ScopeGroup> scopeGroups{};

		Array<SwapChain*> presentSwapChains{};
		Array<Scope*> presentingScopes{};

		u32 numFramesInFlight = 1;
		
		Array<Scope*> producers{};
		Array<Scope*> endScopes{};
		Scope* currentScope = nullptr;

    };

} // namespace CE::RHI
