#include "Fusion.h"

#include "VulkanRHI.h"

namespace CE
{

	FusionStandaloneApplication::FusionStandaloneApplication()
	{
		Init();
	}

	FusionStandaloneApplication::~FusionStandaloneApplication()
	{
		Shutdown();
	}

	void FusionStandaloneApplication::Run(Ref<FWindow> mainWindow, u32 width, u32 height,
		const PlatformWindowInfo& windowInfo)
	{
		
	}

	void FusionStandaloneApplication::CreateRHI()
	{
		RHI::gDynamicRHI = new Vulkan::VulkanRHI();
	}

	void FusionStandaloneApplication::SetupDefaultStyle()
	{
		FusionApplication* fApp = FusionApplication::Get();
		FStyleManager* styleManager = fApp->GetStyleManager();

		FRootContext* rootContext = fApp->GetRootContext();

		rootStyle = CreateObject<FStyleSet>(rootContext, "RootStyleSet");
		styleManager->RegisterStyleSet(rootStyle->GetName(), rootStyle);
		rootContext->SetDefaultStyleSet(rootStyle);

		// - Styling -
		{
			{
				auto primaryBtn = CreateObject<FCustomButtonStyle>(rootStyle, "PrimaryButton");
				rootStyle->Add("Button.Primary", primaryBtn);

				primaryBtn->background = Color::RGBA(56, 56, 56);
				primaryBtn->hoveredBackground = Color::RGBA(95, 95, 95);
				primaryBtn->pressedBackground = Color::RGBA(50, 50, 50);
				primaryBtn->cornerRadius = Vec4(1, 1, 1, 1) * 5;
				primaryBtn->borderColor = Color::RGBA(24, 24, 24);
				primaryBtn->hoveredBorderColor = primaryBtn->pressedBorderColor = primaryBtn->borderColor;
				primaryBtn->borderWidth = 1.5f;

				GetDefaultWidget<FButton>()
					.Padding(Vec4(10, 5, 10, 5))
					.Style(rootStyle, "Button.Primary")
					;
				GetDefaultWidget<FTextButton>()
					.Padding(Vec4(10, 5, 10, 5))
					.Style(rootStyle, "Button.Primary")
					;

				GetDefaultWidget<FScrollBox>()
					.ScrollBarBackground(Color::RGBA(50, 50, 50))
					.ScrollBarBrush(Color::RGBA(130, 130, 130))
					.ScrollBarHoverBrush(Color::RGBA(160, 160, 160))
					;
			}

			{
				auto toolWindow = CreateObject<FToolWindowStyle>(rootStyle, "ToolWindow");
				rootStyle->Add(toolWindow);

				GetDefaultWidget<FToolWindow>()
					.Style(rootStyle, "ToolWindow");
			}

			{
				FWindowControlButtonStyle* windowCloseButton = nullptr;
				FWindowControlButtonStyle* windowControlButton = nullptr;

				windowCloseButton = CreateObject<FWindowControlButtonStyle>(rootStyle, "Button_WindowClose");
				rootStyle->Add("Button.WindowClose", windowCloseButton);

				windowCloseButton->background = Colors::Clear;
				windowCloseButton->hoveredBackground = Color::RGBA(161, 57, 57);
				windowCloseButton->hoveredBackground = Color::RGBA(196, 43, 28);
				windowCloseButton->pressedBackground = Color::RGBA(178, 43, 30);
				windowCloseButton->borderColor = Colors::Clear;
				windowCloseButton->borderWidth = 0.0f;
				windowCloseButton->cornerRadius = Vec4();
				windowCloseButton->contentMoveY = 0;

				windowControlButton = CreateObject<FWindowControlButtonStyle>(rootStyle, "Button_WindowControl");
				rootStyle->Add("Button.WindowControl", windowControlButton);

				windowControlButton->background = Colors::Clear;
				windowControlButton->hoveredBackground = Color::RGBA(58, 58, 58);
				windowControlButton->pressedBackground = Color::RGBA(48, 48, 48);
				windowControlButton->borderColor = Colors::Clear;
				windowControlButton->borderWidth = 0.0f;
				windowControlButton->cornerRadius = Vec4();
				windowControlButton->contentMoveY = 0;
			}

			{
				auto primaryTextInput = CreateObject<FTextInputPlainStyle>(rootStyle, "PrimaryTextInput");
				rootStyle->Add("TextInput.Primary", primaryTextInput);

				primaryTextInput->background = primaryTextInput->hoverBackground = primaryTextInput->editingBackground = Color::RGBA(15, 15, 15);
				primaryTextInput->borderColor = Color::RGBA(60, 60, 60);
				primaryTextInput->editingBorderColor = Color::RGBA(0, 112, 224);
				primaryTextInput->hoverBorderColor = Color::RGBA(74, 74, 74);
				primaryTextInput->borderWidth = 1.0f;
				primaryTextInput->cornerRadius = Vec4(5, 5, 5, 5);

				GetDefaultWidget<FTextInput>()
					.Style(rootStyle, "TextInput.Primary");
			}

			{
				auto primaryComboBox = CreateObject<FComboBoxStyle>(rootStyle, "PrimaryComboBoxStyle");
				rootStyle->Add("ComboBox.Primary", primaryComboBox);

				primaryComboBox->background = Color::RGBA(15, 15, 15);
				primaryComboBox->borderColor = Color::RGBA(60, 60, 60);
				primaryComboBox->pressedBorderColor = primaryComboBox->hoverBorderColor = Color::RGBA(74, 74, 74);
				primaryComboBox->borderWidth = 1.0f;
				primaryComboBox->cornerRadius = Vec4(5, 5, 5, 5);

				auto primaryComboBoxItem = CreateObject<FComboBoxItemStyle>(rootStyle, "PrimaryComboBoxItemStyle");
				rootStyle->Add("ComboBoxItem.Primary", primaryComboBoxItem);

				primaryComboBoxItem->background = Colors::Clear;
				primaryComboBoxItem->hoverBackground = Color::RGBA(0, 112, 224);
				primaryComboBoxItem->selectedBackground = Colors::Clear;
				primaryComboBoxItem->selectedShape = FShapeType::RoundedRect;
				primaryComboBoxItem->shapeCornerRadius = Vec4(1, 1, 1, 1) * 3;
				primaryComboBoxItem->selectedBorderColor = primaryComboBoxItem->hoverBackground.GetFillColor();
				primaryComboBoxItem->borderWidth = 1.0f;

				GetDefaultWidget<FComboBox>()
					.ItemStyle(primaryComboBoxItem)
					.Style(rootStyle, "ComboBox.Primary");

				auto primaryComboBoxPopup = CreateObject<FComboBoxPopupStyle>(rootStyle, "PrimaryComboBoxPopupStyle");
				rootStyle->Add("ComboBoxPopup.Primary", primaryComboBoxPopup);

				primaryComboBoxPopup->background = Color::RGBA(26, 26, 26);
				primaryComboBoxPopup->borderWidth = 0.0f;

				GetDefaultWidget<FComboBoxPopup>()
					.Style(rootStyle, "ComboBoxPopup.Primary");
			}

			{
				auto primaryTabView = CreateObject<FTabViewStyle>(rootStyle, "TabViewStyle");
				rootStyle->Add("TabView.Primary", primaryTabView);

				primaryTabView->tabItemActiveBackground = Color::RGBA(50, 50, 50);
				primaryTabView->tabItemHoverBackground = Color::RGBA(40, 40, 40);
				primaryTabView->tabItemBackground = Color::RGBA(36, 36, 36);
				primaryTabView->tabItemShape = FRoundedRectangle(3, 3, 0, 0);

				GetDefaultWidget<FTabView>()
					.Style(rootStyle, "TabView.Primary");
			}

			{
				auto menuStyle = CreateObject<FMenuPopupStyle>(rootStyle, "MenuPopupStyle");
				rootStyle->Add("MenuPopup.Primary", menuStyle);

				menuStyle->background = Color::RGBA(56, 56, 56);
				menuStyle->itemPadding = Vec4(10, 5, 10, 5);
				menuStyle->itemHoverBackground = Color::RGBA(0, 112, 224);

				GetDefaultWidget<FMenuPopup>()
					.Style(rootStyle, "MenuPopup.Primary");

				auto menuBarStyle = CreateObject<FMenuBarStyle>(rootStyle, "MenuBarStyle");
				rootStyle->Add("MenuBar.Primary", menuBarStyle);

				menuBarStyle->background = Colors::Black;
				menuBarStyle->itemPadding = Vec4(5, 5, 5, 5);
				menuBarStyle->itemHoverBackground = Color::RGBA(0, 112, 224);

				GetDefaultWidget<FMenuBar>()
					.Style(rootStyle, "MenuBar.Primary");
			}

			{

				auto comboBox = CreateObject<FComboBoxStyle>(rootStyle, "ComboBox");
				rootStyle->Add("ComboBox", comboBox);

				comboBox->background = Color::RGBA(15, 15, 15);
				comboBox->borderColor = Color::RGBA(60, 60, 60);
				comboBox->pressedBorderColor = comboBox->hoverBorderColor = Color::RGBA(74, 74, 74);
				comboBox->borderWidth = 1.0f;
				comboBox->cornerRadius = Vec4(5, 5, 5, 5);

				auto comboBoxItem = CreateObject<FComboBoxItemStyle>(rootStyle, "ComboBoxItem");
				rootStyle->Add("ComboBoxItem", comboBoxItem);

				comboBoxItem->background = Colors::Clear;
				comboBoxItem->hoverBackground = Color::RGBA(0, 112, 224);
				comboBoxItem->selectedBackground = Colors::Clear;
				comboBoxItem->selectedShape = FShapeType::RoundedRect;
				comboBoxItem->shapeCornerRadius = Vec4(1, 1, 1, 1) * 3;
				comboBoxItem->selectedBorderColor = Color::RGBA(0, 112, 224);
				comboBoxItem->borderWidth = 1.0f;

				GetDefaultWidget<FComboBox>()
					.ItemStyle(comboBoxItem)
					.Style(comboBox);
			}

			{
				auto treeView = CreateObject<FTreeViewStyle>(rootStyle, "TreeView");
				rootStyle->Add(treeView);

				GetDefaultWidget<FTreeView>()
					.Style(rootStyle, treeView->GetName());
			}

			{
				auto expandCaretButton = CreateObject<FImageButtonStyle>(rootStyle, "ExpandCaretButton");
				rootStyle->Add(expandCaretButton);

				expandCaretButton->tintColor = Color::RGBA(134, 134, 134);
				expandCaretButton->hoverTintColor = Color::RGBA(190, 190, 190);
				expandCaretButton->pressedTintColor = Color::RGBA(180, 180, 180);
			}

			{
				auto closeButton = CreateObject<FImageButtonStyle>(rootStyle, "Button_CloseTab");
				rootStyle->Add("Button.CloseTab", closeButton);

				closeButton->tintColor = Color::RGBA(120, 120, 120);
				closeButton->hoverTintColor = Color::RGBA(180, 180, 180);
				closeButton->pressedTintColor = closeButton->tintColor;
			}

			{
				auto listView = CreateObject<FListViewStyle>(rootStyle, "ListView");
				rootStyle->Add(listView);

				GetDefaultWidget<FListView>()
					.Style(rootStyle, listView->GetName());
			}

			{
				auto dockspaceStyle = CreateObject<FDockspaceStyle>(rootStyle, "Dockspace");
				dockspaceStyle->background = Color::RGBA(26, 26, 26);

				dockspaceStyle->tabWellStyle = CreateObject<FDockTabWellStyle>(rootStyle, "DockTabWell");
				rootStyle->Add(dockspaceStyle);
				rootStyle->Add(dockspaceStyle->tabWellStyle.Get());

				GetDefaultWidget<FDockspace>()
					.Style(rootStyle, dockspaceStyle->GetName());

				GetDefaultWidget<FDockTabWell>()
					.Style(rootStyle, dockspaceStyle->tabWellStyle->GetName());
			}

			{
				auto dockspaceStyle = CreateObject<FDockspaceStyle>(rootStyle, "MinorDockspace");
				dockspaceStyle->background = Color::RGBA(26, 26, 26);

				dockspaceStyle->tabWellStyle = CreateObject<FDockTabWellStyle>(rootStyle, "MinorDockTabWell");

				rootStyle->Add(dockspaceStyle);
				rootStyle->Add(dockspaceStyle->tabWellStyle.Get());
			}
		}
	}

	void FusionStandaloneApplication::Init()
	{
		gProjectPath = PlatformDirectories::GetLaunchDir();
		gProjectName = "FusionStandaloneApplication";

		ModuleManager::Get().LoadModule("Core");
		ModuleManager::Get().LoadModule("CoreApplication");
		ModuleManager::Get().LoadModule("CoreInput");
		ModuleManager::Get().LoadModule("CoreMedia");
		ModuleManager::Get().LoadModule("CoreShader");
		ModuleManager::Get().LoadModule("CoreRHI");
		ModuleManager::Get().LoadModule("VulkanRHI");
		ModuleManager::Get().LoadModule("CoreRPI");
		ModuleManager::Get().LoadModule("FusionCore");
		ModuleManager::Get().LoadModule("Fusion");

		PlatformApplication* app = PlatformApplication::Get();

		app->Initialize();
	}

	void FusionStandaloneApplication::Shutdown()
	{
		PlatformApplication::Get()->RemoveMessageHandler(this);

		Logger::Shutdown();

		jobManager->DeactivateWorkersAndWait();

		JobContext::PopGlobalContext();
		delete jobContext;
		jobContext = nullptr;
		delete jobManager;
		jobManager = nullptr;

		FusionApplication* fApp = FusionApplication::Get();

		fApp->PreShutdown();
		fApp->Shutdown();
		fApp->BeginDestroy();

		delete RHI::FrameScheduler::Get();

		RPI::RPISystem::Get().Shutdown();

		RHI::gDynamicRHI->PreShutdown();
		RHI::gDynamicRHI->Shutdown();
		delete RHI::gDynamicRHI; RHI::gDynamicRHI = nullptr;

		PlatformApplication* app = PlatformApplication::Get();

		InputManager::Get().Shutdown(app);

		app->PreShutdown();
		app->Shutdown();
		delete app;

		ModuleManager::Get().UnloadModule("CoreRPI");
		ModuleManager::Get().UnloadModule("FusionCore");
		ModuleManager::Get().UnloadModule("Fusion");
		ModuleManager::Get().UnloadModule("VulkanRHI");
		ModuleManager::Get().UnloadModule("CoreRHI");
		ModuleManager::Get().UnloadModule("CoreShader");
		ModuleManager::Get().UnloadModule("CoreMedia");
		ModuleManager::Get().UnloadModule("CoreInput");
		ModuleManager::Get().UnloadModule("CoreApplication");
		ModuleManager::Get().UnloadModule("Core");
	}

	void FusionStandaloneApplication::Render()
	{
		FusionApplication* app = FusionApplication::TryGet();

		int submittedImageIndex = -1;

		if (app)
		{
			app->Tick();
		}

		if (IsEngineRequestingExit())
		{
			return;
		}

		if (rebuildFrameGraph)
		{
			rebuildFrameGraph = false;
			recompileFrameGraph = true;

			BuildFrameGraph();
			submittedImageIndex = curImageIndex;
		}

		if (recompileFrameGraph)
		{
			recompileFrameGraph = false;

			CompileFrameGraph();
		}

		if (IsEngineRequestingExit())
		{
			return;
		}

		auto scheduler = FrameScheduler::Get();

		if (rebuildFrameGraph || recompileFrameGraph)
		{
			RebuildFrameGraph();
			return;
		}

		int imageIndex = scheduler->BeginExecution();

		if (imageIndex >= RHI::Limits::MaxSwapChainImageCount || rebuildFrameGraph || recompileFrameGraph)
		{
			RebuildFrameGraph();
			return;
		}

		curImageIndex = imageIndex;

		// ---------------------------------------------------------
		// - Enqueue draw packets to views

		if (submittedImageIndex != curImageIndex)
		{
			RPI::RPISystem::Get().SimulationTick(curImageIndex);
			RPI::RPISystem::Get().RenderTick(curImageIndex);
		}

		// ---------------------------------------------------------
		// - Submit draw lists to scopes for execution

		drawList.Shutdown();

		RHI::DrawListMask drawListMask{};
		HashSet<RHI::DrawListTag> drawListTags{};

		// - Setup draw list mask

		if (app)
		{
			app->UpdateDrawListMask(drawListMask);
		}

		// - Enqueue additional draw packets

		for (int i = 0; i < drawListMask.GetSize(); ++i)
		{
			if (drawListMask.Test(i))
			{
				drawListTags.Add((u8)i);
			}
		}

		drawList.Init(drawListMask);

		if (app)
		{
			app->EnqueueDrawPackets(drawList, curImageIndex);
		}

		drawList.Finalize();

		// - Set scope draw lists

		if (app) // FWidget Scopes & DrawLists
		{
			app->FlushDrawPackets(drawList, curImageIndex);
		}


		scheduler->EndExecution();
	}

	void FusionStandaloneApplication::RebuildFrameGraph()
	{
		rebuildFrameGraph = recompileFrameGraph = true;
	}

	void FusionStandaloneApplication::BuildFrameGraph()
	{
		rebuildFrameGraph = false;
		recompileFrameGraph = true;

		RPI::RPISystem::Get().SimulationTick(curImageIndex);
		RPI::RPISystem::Get().RenderTick(curImageIndex);

		auto scheduler = RHI::FrameScheduler::Get();

		RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

		scheduler->BeginFrameGraph();
		{
			auto app = FusionApplication::TryGet();

			if (app)
			{
				app->EmplaceFrameAttachments();

				app->EnqueueScopes();
			}
		}
		scheduler->EndFrameGraph();
	}

	void FusionStandaloneApplication::CompileFrameGraph()
	{
		recompileFrameGraph = false;

		auto scheduler = RHI::FrameScheduler::Get();

		scheduler->Compile();

		RHI::TransientMemoryPool* pool = scheduler->GetTransientPool();
		RHI::MemoryHeap* imageHeap = pool->GetImagePool();
	}

	void FusionStandaloneApplication::OnWindowRestored(PlatformWindow* window)
	{
		RebuildFrameGraph();
	}

	void FusionStandaloneApplication::OnWindowDestroyed(PlatformWindow* window)
	{
		RebuildFrameGraph();
	}

	void FusionStandaloneApplication::OnWindowClosed(PlatformWindow* window)
	{
		RebuildFrameGraph();
	}

	void FusionStandaloneApplication::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
	{
		RebuildFrameGraph();
	}

	void FusionStandaloneApplication::OnWindowMinimized(PlatformWindow* window)
	{
		RebuildFrameGraph();
	}

	void FusionStandaloneApplication::OnWindowCreated(PlatformWindow* window)
	{
		RebuildFrameGraph();
	}

	void FusionStandaloneApplication::OnWindowExposed(PlatformWindow* window)
	{
		auto id = window->GetWindowId();
		Vec2i windowSize = window->GetWindowSize();

		if (!windowSizesById.KeyExists(id) || windowSize != windowSizesById[id])
		{
			RebuildFrameGraph();

			scheduler->ResetFramesInFlight();
		}

		windowSizesById[id] = windowSize; ApplicationMessageHandler::OnWindowExposed(window);
	}
} // namespace CE
