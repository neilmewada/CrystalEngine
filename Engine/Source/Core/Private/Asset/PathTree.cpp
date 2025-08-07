#include "Core.h"

namespace CE
{
	PathTreeNode::~PathTreeNode()
	{
		if (parent != nullptr)
			parent->children.Remove(this);

		RemoveAll();
	}

	bool PathTreeNode::ChildExistsRecursive(PathTreeNode* child)
	{
		for (PathTreeNode* childNode : children)
		{
			if (childNode == child)
			{
				return true;
			}

			if (ChildExistsRecursive(childNode))
			{
				return true;
			}
		}

		return false;
	}

	bool PathTreeNode::ChildExists(const String& nameStr)
	{
		CE::Name name = nameStr;

		for (auto child : children)
		{
			if (child != nullptr && child->name == name)
				return true;
		}
		return false;
	}

	PathTreeNode* PathTreeNode::GetOrAddChild(const String& name, PathTreeNodeType type, void* userData, u32 userDataSize)
	{
		for (auto child : children)
		{
			if (child != nullptr && child->name == name)
				return child;
		}
		children.Add(new PathTreeNode());
		children.Top()->name = name;
		children.Top()->nodeType = type;
		children.Top()->parent = this;
		children.Top()->userData = userData;
		children.Top()->userDataSize = userDataSize;
		return children.Top();
	}

	PathTreeNode* PathTreeNode::GetChild(const String& name)
	{
		for (auto child : children)
		{
			if (child != nullptr && child->name == name)
				return child;
		}
		return nullptr;
	}

    String PathTreeNode::GetFullPath()
    {
		String path = name.GetString();
		if (!path.StartsWith("/"))
			path = "/" + path;

		if (parent != nullptr)
		{
			path = parent->GetFullPath() + path;
		}

		if (path.StartsWith("//"))
			path = path.GetSubstringView(1);

		return path;
    }

	void PathTreeNode::RemoveAll()
	{
		for (int i = children.GetSize() - 1; i >= 0; i--)
		{
			delete children[i];
		}
		children.Clear();
	}

	void PathTreeNode::Clone(const PathTreeNode& copy)
	{
		name = copy.name;
		nodeType = copy.nodeType;
		userData = copy.userData;
		userDataSize = copy.userDataSize;

		for (auto copyChild : copy.children)
		{
			PathTreeNode* child = new PathTreeNode();
			child->parent = this;
			child->Clone(*copyChild);
			children.Add(child);
		}
	}

	void PathTreeNode::SortChildren(const Delegate<bool(PathTreeNode* lhs, PathTreeNode* rhs)>& lessThanComparison)
	{
		children.Sort([&](PathTreeNode* lhs, PathTreeNode* rhs) -> bool
		{
			return lessThanComparison(lhs, rhs);
		});


	}

	void PathTreeNode::SortChildren()
	{
		SortChildren([](PathTreeNode* lhs, PathTreeNode* rhs) -> bool
		{
			if (lhs->nodeType == rhs->nodeType)
			{
				return String::NaturalCompare(lhs->name.GetString(), rhs->name.GetString());
			}
			else if (lhs->nodeType == PathTreeNodeType::Directory) // lhs is Directory and rhs is Asset
			{
				return true;
			}
			else // lhs is Asset & rhs is Directory
			{
				return false;
			}
		});
	}

	void PathTreeNode::SortChildrenRecursively(const Delegate<bool(PathTreeNode* lhs, PathTreeNode* rhs)>& lessThanComparison)
	{
		SortChildren(lessThanComparison);

		for (PathTreeNode* child : children)
		{
			child->SortChildrenRecursively(lessThanComparison);
		}
	}

	void PathTreeNode::SortChildrenRecursively()
	{
		SortChildren();

		for (PathTreeNode* child : children)
		{
			child->SortChildrenRecursively();
		}
	}

	PathTree::PathTree()
	{
		rootNode = new PathTreeNode();
		rootNode->name = "/";
		rootNode->nodeType = PathTreeNodeType::Directory;
	}

	PathTree::~PathTree()
	{
		delete rootNode;
		rootNode = nullptr;
	}

	PathTreeNode* PathTree::AddPath(const Name& path, void* userData, u32 userDataSize)
	{
		if (!path.IsValid())
			return nullptr;

		const String& pathStr = path.GetString();
		if (!pathStr.StartsWith("/"))
			return nullptr;

		Array<String> components = {};
		pathStr.Split(Array<String>{ "/" , "\\" }, components);

		PathTreeNode* curNode = rootNode;

		for (int i = 0; i < components.GetSize(); i++)
		{
			const String& component = components[i];
			bool isLast = i == components.GetSize() - 1;
			
			if (!isLast || !component.Contains('.'))
			{
				curNode = curNode->GetOrAddChild(component, PathTreeNodeType::Directory);
			}
			else
			{
				curNode = curNode->GetOrAddChild(component, PathTreeNodeType::Asset, userData, userDataSize);
			}
		}

		return curNode;
	}

	PathTreeNode* PathTree::AddPath(const Name& path, PathTreeNodeType nodeType, void* userData, u32 userDataSize)
	{
		if (!path.IsValid())
			return nullptr;

		const String& pathStr = path.GetString();
		if (!pathStr.StartsWith("/"))
			return nullptr;

		Array<String> components = {};
		pathStr.Split(Array<String>{ "/", "\\" }, components);

		PathTreeNode* curNode = rootNode;

		for (int i = 0; i < components.GetSize(); i++)
		{
			const String& component = components[i];

			curNode = curNode->GetOrAddChild(component, nodeType, userData, userDataSize);
		}

		return curNode;
	}

	bool PathTree::RemovePath(const Name& path)
	{
		if (!path.IsValid())
			return false;

		const String& pathStr = path.GetString();
		if (!pathStr.StartsWith("/") || pathStr == "/")
			return false;

		Array<String> components = pathStr.Split('/');

		PathTreeNode* curNode = rootNode;

		for (int i = 0; i < components.GetSize(); i++)
		{
			if (curNode == nullptr)
				return false;

			const String& component = components[i];

			curNode = curNode->GetChild(component);
		}

		if (curNode == rootNode)
			return false;

		delete curNode;
		return true;
	}

	void PathTree::RemoveAll()
	{
		rootNode->RemoveAll();
	}

	PathTreeNode* PathTree::GetNode(const Name& path)
	{
		if (!path.IsValid())
			return nullptr;

		String pathStr = path.GetString();
		if (!pathStr.StartsWith("/"))
			return nullptr;
		if (pathStr == "/")
			return rootNode;

		Array<String> components = pathStr.Split('/');

		PathTreeNode* curNode = rootNode;

		for (int i = 0; i < components.GetSize(); i++)
		{
			if (curNode == nullptr)
				return nullptr;

			const String& component = components[i];

			curNode = curNode->GetChild(component);
		}

		if (curNode == rootNode)
			return nullptr;

		return curNode;
	}

	void PathTree::Sort(const Delegate<bool(PathTreeNode* lhs, PathTreeNode* rhs)>& lessThanComparison)
	{
		rootNode->SortChildrenRecursively(lessThanComparison);
	}

	void PathTree::Sort()
	{
		Sort([](PathTreeNode* lhs, PathTreeNode* rhs) -> bool
		{
			if (lhs->nodeType == rhs->nodeType)
			{
				return String::NaturalCompare(lhs->name.GetString(), rhs->name.GetString());
			}
			else if (lhs->nodeType == PathTreeNodeType::Directory) // lhs is Directory and rhs is Asset
			{
				return true;
			}
			else // lhs is Asset & rhs is Directory
			{
				return false;
			}
		});
	}


} // namespace CE
