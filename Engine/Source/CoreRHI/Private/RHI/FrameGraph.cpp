#include "CoreRHI.h"

namespace CE::RHI
{

    FrameGraph::FrameGraph()
    {
        
    }

    FrameGraph::~FrameGraph()
    {
		DeleteAll();
    }

    void FrameGraph::DeleteAll()
    {
		presentingScopes.Clear();
		presentSwapChains.Clear();
		for (auto scope : scopes)
		{
			delete scope;
		}
		scopes.Clear();
		scopesById.Clear();
		producers.Clear();
		scopeGroups.Clear();
		nodes.Clear();

		attachmentDatabase.Clear();
		lastWrittenAttachmentToScope.Clear();
		attachmentReadSchedule.Clear();
		nodeDependencies.Clear();

		scopesByTimelineLevel.Clear();
		maxTimelineLevel = 0;
    }

	bool FrameGraph::Build()
	{
		ZoneScoped;

		lastWrittenAttachmentToScope.Clear();
		attachmentReadSchedule.Clear();
		nodeDependencies.Clear();

		scopesByTimelineLevel.Clear();
		maxTimelineLevel = 0;

		for (Scope* scope : scopes)
		{
			// Process read dependencies
			for (auto attachment : scope->readAttachments) 
			{
				if (lastWrittenAttachmentToScope.KeyExists(attachment->id))
				{
					AddScopeDependency(lastWrittenAttachmentToScope[attachment->id], scope);
				}
				attachmentReadSchedule[attachment->id].Add(scope);
			}

			// Process write dependencies
			for (const auto& attachment : scope->writeAttachments)
			{
				if (attachmentReadSchedule.KeyExists(attachment->id))
				{
					for (auto readPass : attachmentReadSchedule[attachment->id])
					{
						AddScopeDependency(readPass, scope);
					}
					attachmentReadSchedule[attachment->id].Clear();
				}

				if (lastWrittenAttachmentToScope.KeyExists(attachment->id))
				{
					AddScopeDependency(lastWrittenAttachmentToScope[attachment->id], scope);
				}

				lastWrittenAttachmentToScope[attachment->id] = scope;
			}
		}

		FinalizeGraph();

		return true;
	}

	void FrameGraph::AddScopeDependency(Scope* from, Scope* to)
	{
		if (from != to)
		{
			nodeDependencies[to].Add(from);
		}
	}

	bool FrameGraph::ScopeHasDependency(Scope* source, Scope* dependentOn) 
	{
		if (!nodeDependencies.KeyExists(source))
			return false;

		for (Scope* scope : nodeDependencies[source])
		{
			if (scope == dependentOn)
				return true;
			else if (ScopeHasDependency(scope, dependentOn))
				return true;
		}
		return false;
	}

	void FrameGraph::FinalizeGraph()
	{
		ZoneScoped;

		producers.Clear();
		nodes.Clear();
		endScopes.Clear();

		HashMap<Scope*, HashSet<Scope*>> producersForEachScope{};

		for (auto& [scope, dependencies] : nodeDependencies)
		{
			HashSet<Scope*> producerList = dependencies;

			for (Scope* prod1 : dependencies)
			{
				for (Scope* prod2 : dependencies)
				{
					if (prod1 == prod2)
						continue;

					if (ScopeHasDependency(prod2, prod1))
					{
						producerList.Remove(prod1);
					}
					else if (ScopeHasDependency(prod1, prod2))
					{
						producerList.Remove(prod2);
					}
				}
			}

			producersForEachScope[scope] = producerList;
		}

		for (auto scope : scopes)
		{
			if (!nodeDependencies.KeyExists(scope) || nodeDependencies[scope].IsEmpty())
				producers.Add(scope);
		}

		nodeDependencies = producersForEachScope;

		for (auto& [scope, dependencies] : nodeDependencies)
		{
			for (RHI::Scope* dependent : dependencies)
			{
				if (!nodes.KeyExists(dependent->id))
					nodes.Add(dependent->id, dependent);
				nodes[dependent->id].consumers.Add(scope);
				dependent->consumers.Add(scope);

				if (!nodes.KeyExists(scope->id))
					nodes.Add(scope->id, scope);
				nodes[scope->id].producers.Add(dependent);
				scope->producers.Add(dependent);
			}
		}

		// Form a chain of scopes that don't have any branches
		for (Scope* scope : scopes)
		{
			if (scope->producers.GetSize() == 1 && !scope->producers[0]->PresentsSwapChain())
				scope->prev = scope->producers[0];
			if (scope->consumers.GetSize() == 1 && !scope->PresentsSwapChain() && scope->consumers[0]->producers.GetSize() == 1)
				scope->next = scope->consumers[0];
			if (scope->consumers.IsEmpty())
				endScopes.Add(scope);
		}

		// - Timeline Levels -
		
		std::queue<RHI::Scope*> processQueue;
		for (auto* scope : scopes) {
			scope->timelineLevel = 0;
			// Tracks how many producers we are still waiting on
			scope->remainingProducers = scope->producers.GetSize();

			// Start nodes have 0 producers
			if (scope->remainingProducers == 0) {
				processQueue.push(scope);
			}
		}

		while (!processQueue.empty()) {
			RHI::Scope* current = processQueue.front();
			processQueue.pop();

			for (auto* consumer : current->consumers) {
				// Level is always 1 higher than the furthest producer
				consumer->timelineLevel = std::max(consumer->timelineLevel, current->timelineLevel + 1);

				// Once all producers are visited, this scope is ready to be a producer itself
				consumer->remainingProducers--;
				if (consumer->remainingProducers == 0) {
					processQueue.push(consumer);
				}
			}
		}

		for (auto* scope : scopes) 
		{
			scopesByTimelineLevel[scope->timelineLevel].Add(scope);
			maxTimelineLevel = Math::Max(maxTimelineLevel, scope->timelineLevel);

			// TODO: Track min and max timeline per FrameAttachment for aliasing
		}

#if false

		// For debugging
		FileStream jsonFile = FileStream(PlatformDirectories::GetLaunchDir() / "Temp/FrameGraph.json", Stream::Permissions::WriteOnly);
		
		JValue root = JObject();

		std::function<void(RHI::Scope*, JValue&)> travelScope = [&](RHI::Scope* scope, JValue& root)
			{
				root[scope->GetId().GetString()] = JObject();
				JValue& objectValue = root[scope->GetId().GetString()];
				JObject& object = root[scope->GetId().GetString()].GetObjectValue();
				if (scope->prev != nullptr)
					object["__Prev"] = scope->prev->GetId().GetString();
				if (scope->next != nullptr)
					object["__Next"] = scope->next->GetId().GetString();

				for (auto child : scope->consumers)
				{
					travelScope(child, objectValue);
				}
			};
		
		for (RHI::Scope* producer : producers)
		{
			travelScope(producer, root);
		}

		JsonSerializer::Serialize2(&jsonFile, root);
		jsonFile.Close();
#endif
	}

} // namespace CE::RHI
