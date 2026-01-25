
#include "FusionTest.h"

namespace WidgetTests
{
	// ------------------------------------------------
	// - Major Dockspace

	MajorDockspaceWindow::MajorDockspaceWindow()
	{
		m_DockspaceClass = MajorDockspace::StaticClass();
	}

	void MajorDockspaceWindow::Construct()
	{
		Super::Construct();

	}

	MajorDockspace::MajorDockspace()
	{
		m_DockspaceType = FDockTypeMask::Major;
		m_AllowDocking = true;
		m_AllowSplitting = false;

		detachedDockspaceWindowClass = MajorDockspaceWindow::StaticClass();
	}

	void MajorDockspace::Construct()
	{
		Super::Construct();

		container->GetTabWell()->Padding(Vec4(1, 1.25f, 0, 0) * 15);

		Style("Dockspace");

		OnCreateTabItem([](FDockTabItem& tabItem)
			{
				tabItem.MinWidth(150);
			});
	}

	// ------------------------------------------------
	// - Minor Dockspace

	MinorDockspaceWindow::MinorDockspaceWindow()
	{
		m_DockspaceClass = MinorDockspace::StaticClass();
	}

	void MinorDockspaceWindow::Construct()
	{
		Super::Construct();

		
	}

	MinorDockspace::MinorDockspace()
	{
		m_DockspaceType = FDockTypeMask::Minor;
		m_AllowDocking = true;
		m_AllowSplitting = true;

		detachedDockspaceWindowClass = MinorDockspaceWindow::StaticClass();
	}

	void MinorDockspace::Construct()
	{
		Super::Construct();

		Style("MinorDockspace");

		OnCreateTabItem([](FDockTabItem& tabItem)
			{
				tabItem.MinWidth(120);
			});
	}

	SceneTreeView::SceneTreeView()
	{

	}

	void SceneTreeView::Construct()
	{
		Super::Construct();

		(*this)
		.GenerateRowDelegate(MemberDelegate(&Self::GenerateRow, this))
		.RowContextMenuDelegate(FUNCTION_BINDING(this, BuildRowContextMenu))
			;

		SelectionModel()->SetSelectionMode(FSelectionMode::Extended);

		Style("TreeView");
	}

	FTreeViewRow& SceneTreeView::GenerateRow()
	{
		FTreeViewRow& row = FNew(FTreeViewRow);

		constexpr f32 fontSize = 10;

		row.Cells(
			FNew(FTreeViewCell)
			.Text("Name")
			.ArrowEnabled(true)
			.OnLabelEdited([this](FTreeViewCell& cell, const String& newLabel)
			{
				if (Ref<FTreeViewRow> row = cell.GetRow())
				{
					FModelIndex index = row->GetIndex();
					if (index.IsValid() && index.GetData().HasValue())
					{
						if (PathTreeNode* node = index.GetData().GetValue<PathTreeNode*>())
						{
							node->name = newLabel;
							OnModelUpdate();
						}
					}
				}
			})
			.FontSize(fontSize),

			FNew(FTreeViewCell)
			.Text("Type")
			.FontSize(fontSize)
			.Foreground(Color::RGBA(255, 255, 255, 140))
			.ArrowEnabled(false)
		);

		return row;
	}

	FMenuPopup& SceneTreeView::BuildRowContextMenu(FTreeViewRow& row)
	{
		return
		FNew(FMenuPopup)
		.Content(
			FNew(FMenuItem)
			.Text("Add Child")
			.OnClick([this]
			{
				FModelIndex index = SelectionModel()->GetSelectedIndex();
				if (index.IsValid())
				{
					if (PathTreeNode* node = index.GetData().GetValue<PathTreeNode*>())
					{
						String newName = "Child_0";
						int i = 0;
						while (node->ChildExists(newName))
						{
							newName = String::Format("Child_{}", ++i);
						}

						node->GetOrAddChild(newName, PathTreeNodeType::Directory);

						OnModelUpdate();
					}
				}
			}),

			FNew(FMenuItem)
			.Text("Delete")
			.OnClick([this]
			{
				FModelIndex index = SelectionModel()->GetSelectedIndex();
				if (index.IsValid())
				{
					if (PathTreeNode* node = index.GetData().GetValue<PathTreeNode*>())
					{
						if (PathTreeNode* parent = node->parent)
						{
							parent->children.Remove(node);

							OnModelUpdate();
						}
					}
				}
			}),

			FNew(FMenuItem)
			.Text("Rename")
			.OnClick([this]
			{
				FModelIndex index = SelectionModel()->GetSelectedIndex();
				if (index.IsValid())
				{
					if (Ref<FTreeViewRow> row = FindRow(index))
					{
						row->GetCell(0)->StartEditing();
					}
				}
			})
		)
		.MinWidth(140)
		.As<FMenuPopup>();
	}

	SceneTreeViewModel::SceneTreeViewModel()
	{

	}

	void SceneTreeViewModel::OnBeforeDestroy()
	{
		Super::OnBeforeDestroy();

		delete pathTree; pathTree = nullptr;
	}

	SceneTreeViewModel::~SceneTreeViewModel()
	{

	}

	FModelIndex SceneTreeViewModel::GetIndex(u32 row, u32 column, const FModelIndex& parent)
	{
		if (!pathTree)
			return {};

		if (!parent.IsValid())
		{
			if (row >= pathTree->GetRootNode()->children.GetSize())
				return {};

			return CreateIndex(row, column, pathTree->GetRootNode()->children[row]);
		}

		PathTreeNode* parentNode = parent.GetData().GetValue<PathTreeNode*>();
		if (parentNode == nullptr || row >= parentNode->children.GetSize())
			return {};

		return CreateIndex(row, column, parentNode->children[row]);
	}

	u32 SceneTreeViewModel::GetRowCount(const FModelIndex& parent)
	{
		if (!pathTree)
			return {};

		if (!parent.IsValid())
		{
			return pathTree->GetRootNode()->children.GetSize();
		}

		PathTreeNode* parentNode = parent.GetData().GetValue<PathTreeNode*>();
		if (parentNode == nullptr)
			return {};

		return parentNode->children.GetSize();
	}

	u32 SceneTreeViewModel::GetColumnCount(const FModelIndex& parent)
	{
		return 2; // Name, Type
	}

	void SceneTreeViewModel::SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent)
	{
		if (!pathTree)
			return;

		FTreeViewRow* rowCast = Object::CastTo<FTreeViewRow>(&rowWidget);
		if (rowCast == nullptr)
			return;

		FTreeViewRow& treeRow = *rowCast;

		FModelIndex index = GetIndex(row, 0, parent);
		if (!index.IsValid() || !index.GetData().HasValue())
			return;

		PathTreeNode* node = index.GetData().GetValue<PathTreeNode*>();

		treeRow.GetCell(0)->Text(node->name.GetString());
		treeRow.GetCell(1)->Text((!node->IsTerminal() || node->nodeType == PathTreeNodeType::Directory) ? "Directory" : "File");
	}

	FModelIndex SceneTreeViewModel::GetParent(const FModelIndex& index)
	{
		if (!index.IsValid() || !index.GetData().HasValue())
			return FModelIndex();

		PathTreeNode* node = index.GetData().GetValue<PathTreeNode*>();

		PathTreeNode* parent = node->parent;
		if (parent == nullptr)
			return FModelIndex();

		PathTreeNode* parentsParent = parent->parent;
		if (parentsParent == nullptr)
			return FModelIndex();

		int parentsIndex = parentsParent->children.IndexOf(parent);
		if (parentsIndex == -1)
			return FModelIndex();

		FModelIndex retIndex = CreateIndex(parentsIndex, 0, parent);
		return retIndex;
	}

}
