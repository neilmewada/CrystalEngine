#pragma once

#include "FusionCore.h"

using namespace CE;

namespace RenderingTests
{
	struct BinaryNode : IntrusiveBase
	{
		StaticArray<Ptr<BinaryNode>, 2> child;
		BinaryNode* parent = nullptr;

        int totalChildren = 0;
		Rect rect;
		int imageId = -1;

		bool IsLeaf() const
		{
			return child[0] == nullptr && child[1] == nullptr;
		}

		bool IsWidthSpan() const
		{
			return !IsLeaf() && child[0]->rect.max.x < child[1]->rect.max.x;
		}

		bool IsHeightSpan() const
		{
			return !IsLeaf() && child[0]->rect.max.y < child[1]->rect.max.y;
		}

		bool IsValid() const
		{
			return imageId >= 0;
		}

		bool IsValidRecursive() const
		{
			return IsValid() || (child[0] != nullptr && child[0]->IsValidRecursive()) ||
				(child[1] != nullptr && child[1]->IsValidRecursive());
		}

		Vec2i GetSize() const
		{
			return Vec2i(Math::RoundToInt(rect.GetSize().width), Math::RoundToInt(rect.GetSize().height));
		}

		Ptr<BinaryNode> Insert(Vec2i imageSize);

		void ClearImage()
		{
			imageId = -1;
		}

		void ForEachRecursive(const std::function<void(Ptr<BinaryNode> node)>& visitor)
		{
			visitor(this);

			if (child[0] != nullptr)
			{
				child[0]->ForEachRecursive(visitor);
			}
			if (child[1] != nullptr)
			{
				child[1]->ForEachRecursive(visitor);
			}
		}

		Ptr<BinaryNode> FindUsedNode()
		{
			if (IsValid())
				return this;

			if (child[0] != nullptr)
			{
				Ptr<BinaryNode> node = child[0]->FindUsedNode();
				if (node != nullptr)
				{
					return node;
				}
			}

			if (child[1] != nullptr)
			{
				Ptr<BinaryNode> node = child[1]->FindUsedNode();
				if (node != nullptr)
				{
					return node;
				}
			}

			return nullptr;
		}

		Ptr<BinaryNode> SelectRandom()
		{
			int index = 0;
			return SelectRandomInternal(index, Random::Range(0, totalChildren - 1));
		}

        Ptr<BinaryNode> SelectRandomInternal(int& curIndex, int targetIndex)
		{
			if (curIndex == targetIndex)
			{
				return this;
			}

			if (child[0] != nullptr)
			{
				curIndex++;
				Ptr<BinaryNode> result = child[0]->SelectRandomInternal(curIndex, targetIndex);
				if (result != nullptr)
				{
					return result;
				}
			}

			if (child[1] != nullptr)
			{
				curIndex++;
				Ptr<BinaryNode> result = child[1]->SelectRandomInternal(curIndex, targetIndex);
				if (result != nullptr)
				{
					return result;
				}
			}

			return nullptr;
		}

		bool Defragment();
	};

	inline Ptr<BinaryNode> BinaryNode::Insert(Vec2i imageSize)
	{
		if (child[0] != nullptr)
		{
			// We are not in leaf node
			Ptr<BinaryNode> newNode = child[0]->Insert(imageSize);
			if (newNode != nullptr)
			{
				totalChildren++;
				return newNode;
			}
			if (child[1] == nullptr)
			{
				return nullptr;
			}

			newNode = child[1]->Insert(imageSize);
			if (newNode != nullptr)
			{
				totalChildren++;
			}

			return newNode;
		}
		else // We are in leaf node
		{
			if (imageId >= 0)
				return nullptr;

			if (GetSize().width < imageSize.width ||
				GetSize().height < imageSize.height)
			{
				return nullptr;
			}

			if (GetSize().width == imageSize.width && GetSize().height == imageSize.height)
			{
				return this;
			}

			// Split the node
			child[0] = new BinaryNode;
			child[1] = new BinaryNode;
			child[0]->parent = this;
			child[1]->parent = this;

			totalChildren = 2;

			int dw = GetSize().width - imageSize.width;
			int dh = GetSize().height - imageSize.height;

			if (dw > dh)
			{
				child[0]->rect = Rect(rect.left, rect.top,
					rect.left + imageSize.width, rect.bottom);
				child[1]->rect = Rect(rect.left + imageSize.width + 1, rect.top,
					rect.right, rect.bottom);
			}
			else
			{
				child[0]->rect = Rect(rect.left, rect.top,
					rect.right, rect.top + imageSize.height);
				child[1]->rect = Rect(rect.left, rect.top + imageSize.height + 1,
					rect.right, rect.bottom);
			}

			return child[0]->Insert(imageSize);
		}
	}

	inline bool BinaryNode::Defragment()
	{
		if (child[0] != nullptr && child[1] != nullptr)
		{
			bool leftValid = child[0]->Defragment();
			bool rightValid = child[1]->Defragment();

			if (!leftValid && !rightValid)
			{
				child[0] = nullptr;
				child[1] = nullptr;

				return false;
			}

			// Better defragmentation but very slow:
			//if (false)
			{
				// TODO: It destroys valid rects too. Need to be fixed
				if (IsWidthSpan() && !leftValid && child[1]->child[0] != nullptr &&
					!child[1]->child[0]->IsValidRecursive() && child[1]->IsWidthSpan())
				{
					Ptr<BinaryNode> nodeToMove = child[1]->child[1];
					child[0]->rect.max.x = child[1]->child[0]->rect.max.x;
					child[1] = nodeToMove;
					nodeToMove->parent = this;
				}
				else if (IsHeightSpan() && !leftValid && child[1]->child[0] != nullptr &&
					!child[1]->child[0]->IsValidRecursive() && child[1]->IsHeightSpan())
				{
					Ptr<BinaryNode> nodeToMove = child[1]->child[1];
					child[0]->rect.max.y = child[1]->child[0]->rect.max.y;
					child[1] = nodeToMove;
					nodeToMove->parent = this;
				}
			}

			return true;
		}

		return IsValid();
	}

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

		FUNCTION()
		void OnTimeOut();

		void Construct() override;

		void OnBeginDestroy() override;

		void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowMaximized(PlatformWindow* window) override;
		void OnWindowExposed(PlatformWindow* window) override;

		void OnPaint(FPainter* painter) override;

		FUSION_EVENT(ScriptEvent<void()>, OnAdd);
		FUSION_EVENT(ScriptEvent<void()>, OnRemove);
		FUSION_EVENT(ScriptEvent<void()>, OnDefragment);

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

		FTimer* timer = nullptr;
		f32 gradientAngle = 0;

		int hitCounter = 0;

		FUSION_WIDGET;
	};

}

#include "FusionCoreTest.rtti.h"
