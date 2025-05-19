#pragma once

#include "Fusion.h"
#include "VulkanRHI.h"

using namespace CE;

namespace WidgetTests
{
	class RendererSystem : public ApplicationMessageHandler
	{
		CE_NO_COPY(RendererSystem)
	public:

		static RendererSystem& Get()
		{
			static RendererSystem instance{};
			return instance;
		}

		void Init();
		void Shutdown();

		void Render();

		FUNCTION()
		void RebuildFrameGraph();

		void BuildFrameGraph();
		void CompileFrameGraph();

		void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowDestroyed(PlatformWindow* window) override;
		void OnWindowClosed(PlatformWindow* window) override;
		void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;
		void OnWindowMinimized(PlatformWindow* window) override;
		void OnWindowCreated(PlatformWindow* window) override;
		void OnWindowExposed(PlatformWindow* window) override;

	private:

		RendererSystem() {}

	public:

		HashMap<u64, Vec2i> windowSizesById;
		RHI::FrameScheduler* scheduler = nullptr;

		bool rebuildFrameGraph = true;
		bool recompileFrameGraph = true;
		int curImageIndex = 0;

		RHI::DrawListContext drawList{};
	};

	CLASS()
	class RenderingTestWidget : public FWindow, public ApplicationMessageHandler
	{
		CE_CLASS(RenderingTestWidget, FWindow)
	public:

		RenderingTestWidget() = default;

		void Construct() override;

		FTreeViewRow& GenerateTreeViewRow();

		FListViewRow& GenerateListViewRow();

		void OnBeginDestroy() override;

		void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowMaximized(PlatformWindow* window) override;
		void OnWindowExposed(PlatformWindow* window) override;

		void OnPaint(FPainter* painter) override;

		FSplitBox* splitBox = nullptr;
		FStackBox* rootBox = nullptr;
		FButton* button = nullptr;
		FTextInput* textInput = nullptr;
		FComboBox* comboBox = nullptr;
		FLabel* buttonLabel = nullptr;
		FStyledWidget* subWidget = nullptr;
		FPopup* btnPopup = nullptr;
		FPopup* nativePopup = nullptr;
		FImage* maximizeIcon = nullptr;
		FStyledWidget* borderWidget = nullptr;
		FTextInput* modelTextInput = nullptr;
		FVerticalStack* windowContent = nullptr;
		class TreeViewModel* treeViewModel = nullptr;
		class ListViewModel* listViewModel = nullptr;

		int hitCounter = 0;

		FUSION_WIDGET;
	};

	CLASS()
	class ListViewModel : public FListViewModel
	{
		CE_CLASS(ListViewModel, FListViewModel)
	public:

		ListViewModel()
		{
		}

		int GetRowCount() override
		{
			return 100;
		}

		void SetData(int row, FListViewRow& widget) override
		{
			FVerticalStack& parent = widget.GetChild()->As<FVerticalStack>();

			FLabel& title = parent.GetChild(0)->As<FLabel>();
			title.Text(String::Format("Item {}", row));

			FLabel& description = parent.GetChild(1)->As<FLabel>();
			description.Text(String::Format("This is item number {}.", row));
		}

	};

	CLASS()
	class TreeViewModel : public FAbstractItemModel
	{
		CE_CLASS(TreeViewModel, FAbstractItemModel)
	public:

		TreeViewModel()
		{
			model = new PathTree;

			model->AddPath("/Assets");
			model->AddPath("/Assets/Samples");
			model->AddPath("/Assets/Samples/Splash");
			model->AddPath("/Assets/Samples/Icon");
			model->AddPath("/Assets/Textures");
			model->AddPath("/Assets/Sprites");
			model->AddPath("/Scripts");
			model->AddPath("/Engine");
			model->AddPath("/Editor");
			model->AddPath("/Game");
			model->AddPath("/Temp");
			model->AddPath("/Cache");
			model->AddPath("/Library");
			model->AddPath("/Materials");
			model->AddPath("/Shaders");
		}

		virtual ~TreeViewModel()
		{
			delete model; model = nullptr;
		}

		FModelIndex GetParent(const FModelIndex& index) override
		{
			if (!index.IsValid() || !index.GetData().HasValue())
				return {};

			PathTreeNode* node = (PathTreeNode*)index.GetData().GetValue<PathTreeNode*>();
			if (node->parent == nullptr)
				return {};

			PathTreeNode* parentsParent = node->parent->parent;
			if (parentsParent == nullptr)
				parentsParent = model->GetRootNode();

			int indexOfParent = parentsParent->children.IndexOf(node->parent);
			if (indexOfParent < 0)
				return {};

			return CreateIndex(indexOfParent, index.GetColumn(), node->parent);
		}

		FModelIndex GetIndex(u32 row, u32 column, const FModelIndex& parent) override
		{
			if (!parent.IsValid())
			{
				return CreateIndex(row, column, model->GetRootNode()->children[row]);
			}

			PathTreeNode* node = (PathTreeNode*)parent.GetData().GetValue<PathTreeNode*>();
			if (node == nullptr)
				return {};

			return CreateIndex(row, column, node->children[row]);
		}

		u32 GetRowCount(const FModelIndex& parent) override
		{
			if (!parent.IsValid())
			{
				return model->GetRootNode()->children.GetSize();
			}

			PathTreeNode* node = (PathTreeNode*)parent.GetData().GetValue<PathTreeNode*>();
			if (node == nullptr)
				return 0;

			return node->children.GetSize();
		}

		u32 GetColumnCount(const FModelIndex& parent) override
		{
			return 2;
		}

		void SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent) override
		{
			PathTreeNode* node = nullptr;
			if (!parent.IsValid())
			{
				node = model->GetRootNode()->children[row];
			}
			else
			{
				PathTreeNode* parentNode = (PathTreeNode*)parent.GetData().GetValue<PathTreeNode*>();

				if (parentNode != nullptr)
				{
					node = parentNode->children[row];
				}
			}

			if (node == nullptr)
				return;

			FTreeViewRow& treeRow = rowWidget.As<FTreeViewRow>();

			treeRow.GetCell(0)->Text(node->name.GetString());
			treeRow.GetCell(1)->Text(node->nodeType == PathTreeNodeType::Directory ? "Directory" : "File");
		}

		PathTree* model = nullptr;
	};

}

#include "FusionTest.rtti.h"
