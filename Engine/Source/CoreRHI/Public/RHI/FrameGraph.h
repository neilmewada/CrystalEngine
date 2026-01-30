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
	class FrameGraphVariable;

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

    private:

		void Clear();
        
		bool Build();

		inline void SetVariable(int imageIndex, const Name& name, const FrameGraphVariable& value)
		{
			frameGraphVariables[name][imageIndex] = value;
		}

		inline void SetVariable(const Name& name, const FrameGraphVariable& value)
		{
			for (int i = 0; i < frameGraphVariables[name].GetSize(); i++)
			{
				frameGraphVariables[name][i] = value;
			}
		}

		inline const FrameGraphVariable& GetVariable(int imageIndex, const Name& name)
		{
			return frameGraphVariables[name][imageIndex];
		}

		inline bool VariableExists(const Name& name) const { return frameGraphVariables.KeyExists(name); }

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

		HashMap<Name, StaticArray<FrameGraphVariable, RHI::Limits::MaxSwapChainImageCount>> frameGraphVariables{};

        //! A database of all attachments used in this frame graph.
        FrameAttachmentDatabase attachmentDatabase{};

        Array<Scope*> scopes{};
		HashMap<ScopeId, Scope*> scopesById{};

		//! Multiple scopes can be grouped together as subpasses in a single render pass.
		Array<ScopeGroup> scopeGroups{};

		Array<SwapChain*> presentSwapChains{};
		Array<Scope*> presentingScopes{};

		u32 numFramesInFlight = 1;
		
		Array<Scope*> producers{};
		Array<Scope*> endScopes{};
		Scope* currentScope = nullptr;

        friend class FrameAttachmentDatabase;
		friend class FrameGraphCompiler;
		friend class FrameGraphBuilder;
		friend class FrameScheduler;
		friend class FrameGraphExecuter;
        friend class CE::Vulkan::FrameGraphCompiler;
		friend class CE::Vulkan::FrameGraphExecuter;
        friend class CE::Metal::FrameGraphCompiler;
        friend class CE::Metal::FrameGraphExecuter;
		friend class ::RHI_FrameGraphBuilder_Test;
    };

} // namespace CE::RHI
