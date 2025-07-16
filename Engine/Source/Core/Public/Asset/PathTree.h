#pragma once

namespace CE
{
	enum class PathTreeNodeType
	{
		Directory = 0,
		Asset
	};

	struct CORE_API PathTreeNode
	{
		PathTreeNode() = default;
		virtual ~PathTreeNode();

		PathTreeNode(const PathTreeNode& copy)
		{
			Clone(copy);
		}

		inline PathTreeNode& operator=(const PathTreeNode& copy)
		{
			Clone(copy);
			return *this;
		}

		bool ChildExistsRecursive(PathTreeNode* child);

		bool ChildExists(const String& name);

		PathTreeNode* GetOrAddChild(const String& name, PathTreeNodeType type = PathTreeNodeType::Directory,
			void* userData = nullptr, u32 userDataSize = 0);

		PathTreeNode* GetChild(const String& name);

		String GetFullPath();

		void RemoveAll();

		void Clone(const PathTreeNode& copy);

		void SortChildren(const Delegate<bool(PathTreeNode* lhs, PathTreeNode* rhs)>& lessThanComparison);
		void SortChildren();

		void SortChildrenRecursively(const Delegate<bool(PathTreeNode* lhs, PathTreeNode* rhs)>& lessThanComparison);
		void SortChildrenRecursively();

		inline bool IsTerminal() const
		{
			return children.IsEmpty();
		}

		Name name{};
		PathTreeNodeType nodeType{};

		Array<PathTreeNode*> children{};
		PathTreeNode* parent = nullptr;

		void* userData = nullptr;
		u32 userDataSize = 0;
	};
    
	class CORE_API PathTree
	{
	public:

		PathTree();
		virtual ~PathTree();

		PathTreeNode* AddPath(const Name& path, void* userData = nullptr, u32 userDataSize = 0);
		PathTreeNode* AddPath(const Name& path, PathTreeNodeType nodeType, void* userData = nullptr, u32 userDataSize = 0);

		template<class TFunc>
		void IterateNodesRecursively(const TFunc& func, PathTreeNode* parent = nullptr)
		{
			if (!rootNode)
				return;

			if (parent == nullptr)
				parent = rootNode;

			for (PathTreeNode* child : parent->children)
			{
				func(child);
				IterateNodesRecursively(func, child);
			}
		}

		bool RemovePath(const Name& path);

		void RemoveAll();

		PathTreeNode* GetNode(const Name& path);

		inline PathTreeNode* GetRootNode() const { return rootNode; }

		void Sort(const Delegate<bool(PathTreeNode* lhs, PathTreeNode* rhs)>& lessThanComparison);

		void Sort();

	private:

		PathTreeNode* rootNode = nullptr;
	};

} // namespace CE
