
#include "FusionCore.h"
#include "VulkanRHI.h"

#include "FusionCoreTest.h"

#include <gtest/gtest.h>

#include "FusionCore_Test.private.h"

#define TEST_BEGIN TestBegin(false)
#define TEST_END TestEnd(false)

#define TEST_BEGIN_GUI TestBegin(true)
#define TEST_END_GUI TestEnd(true)

static CE::JobManager* gJobManager = nullptr;
static CE::JobContext* gJobContext = nullptr;

using namespace CE;
using namespace RenderingTests;

static int windowWidth = 0;
static int windowHeight = 0;

static void TestBegin(bool gui)
{
	gProjectPath = PlatformDirectories::GetLaunchDir();
	gProjectName = MODULE_NAME;

	ModuleManager::Get().LoadModule("Core");
	ModuleManager::Get().LoadModule("CoreApplication");
	ModuleManager::Get().LoadModule("CoreInput");
	ModuleManager::Get().LoadModule("CoreMedia");
	ModuleManager::Get().LoadModule("CoreShader");
	ModuleManager::Get().LoadModule("CoreRHI");
	ModuleManager::Get().LoadModule("VulkanRHI");
	ModuleManager::Get().LoadModule("CoreRPI");
	ModuleManager::Get().LoadModule("FusionCore");
	CERegisterModuleTypes();

	PlatformApplication* app = PlatformApplication::Get();

	app->Initialize();

	if (gui)
	{
		PlatformWindowInfo windowInfo{};
		windowInfo.fullscreen = windowInfo.hidden = windowInfo.maximised = windowInfo.resizable = false;
		windowInfo.resizable = true;
		windowInfo.hidden = true;
		windowInfo.windowFlags = PlatformWindowFlags::DestroyOnClose;

		f32 scaleFactor = GetDefaults<FusionApplication>()->GetDefaultScalingFactor();

#if PLATFORM_MAC
		u32 w = 540, h = 400;
#elif PLATFORM_LINUX
		u32 w = 1024 * scaleFactor, h = 768 * scaleFactor;
#elif PLATFORM_WINDOWS
		u32 w = 1024, h = 768;
#endif

		windowWidth = w;
		windowHeight = h;

		PlatformWindow* window = app->InitMainWindow("MainWindow", w, h, windowInfo);
		window->SetBorderless(true);

		InputManager::Get().Initialize(app);
	}

	RHI::gDynamicRHI = new Vulkan::VulkanRHI();

	RHI::gDynamicRHI->Initialize();
	RHI::gDynamicRHI->PostInitialize();

	if (gui)
	{
		RHI::FrameSchedulerDescriptor desc{};
		desc.numFramesInFlight = 2;
		RHI::FrameScheduler::Create(desc);
	}

	RPI::RPISystem::Get().Initialize();

	FusionApplication* fApp = FusionApplication::Get();

	FusionInitInfo initInfo = {};
	initInfo.assetLoader = nullptr;
	fApp->Initialize(initInfo);

	RendererSystem::Get().Init();

	JobManagerDesc desc{};
	desc.defaultTag = JOB_THREAD_WORKER;
	desc.totalThreads = 0; // auto set optimal number of threads
	desc.threads.Resize(4, { .threadName = "FusionThread", .tag = JOB_THREAD_FUSION });

	gJobManager = new JobManager("JobSystemManager", desc);
	gJobContext = new JobContext(gJobManager);
	JobContext::PushGlobalContext(gJobContext);

	Logger::Initialize();
}
static void TestEnd(bool gui)
{
	Logger::Shutdown();

	gJobManager->DeactivateWorkersAndWait();

	JobContext::PopGlobalContext();
	delete gJobContext;
	gJobContext = nullptr;
	delete gJobManager;
	gJobManager = nullptr;

	RendererSystem::Get().Shutdown();

	FusionApplication* fApp = FusionApplication::Get();

	fApp->PreShutdown();
	fApp->Shutdown();
	fApp->BeginDestroy();
	
	if (gui)
	{
		delete RHI::FrameScheduler::Get();
	}

	RPI::RPISystem::Get().Shutdown();

	RHI::gDynamicRHI->PreShutdown();
	RHI::gDynamicRHI->Shutdown();
	delete RHI::gDynamicRHI; RHI::gDynamicRHI = nullptr;

	PlatformApplication* app = PlatformApplication::Get();

	if (gui)
	{
		InputManager::Get().Shutdown(app);
	}

	app->PreShutdown();
	app->Shutdown();
	delete app;

	CEDeregisterModuleTypes();
	ModuleManager::Get().UnloadModule("CoreRPI");
	ModuleManager::Get().UnloadModule("FusionCore");
	ModuleManager::Get().UnloadModule("VulkanRHI");
	ModuleManager::Get().UnloadModule("CoreRHI");
	ModuleManager::Get().UnloadModule("CoreShader");
	ModuleManager::Get().UnloadModule("CoreMedia");
	ModuleManager::Get().UnloadModule("CoreInput");
	ModuleManager::Get().UnloadModule("CoreApplication");
	ModuleManager::Get().UnloadModule("Core");
}

void System(const std::string& filename)
{
#ifdef _WIN32
  system(filename.c_str());
#else
  system(("firefox " + filename).c_str());
#endif
}

static Array<Color> colors = {
	Color::RGBHex(0x37b04e),
	Color::RGBHex(0xcd4e92),
	Color::RGBHex(0x5d5bc5),
	Color::RGBHex(0x115eee),
	Color::RGBHex(0x2cfb66),
	Color::RGBHex(0x34823c),
	Color::RGBHex(0xbd0a57),
	Color::RGBHex(0xd3d5e4),
	Color::RGBHex(0x5b78b3),
	Color::RGBHex(0x1ee740),
	Color::RGBHex(0x34823c),
	Color::RGBHex(0x34823c),
	Color::RGBHex(0xf86210),
	Color::RGBHex(0xd388af),
	Color::RGBHex(0x5b78b3),
	Color::RGBHex(0x2b1152),
	Color::RGBHex(0x26a3f9),
};

static Array<Vec2i> rects = {
	Vec2i(20, 30),
	Vec2i(60, 20),
	Vec2i(30, 30),
	Vec2i(100, 50),
	Vec2i(50, 100),
	Vec2i(200, 100),
	Vec2i(80, 20),
	Vec2i(20, 60),
};

Ptr<FImageAtlas::BinaryNode> root = nullptr;

static void AddRect()
{
	Vec2i imageSize = Vec2i(Random::Range(10, 80), Random::Range(10, 80));
	auto node = root->Insert(imageSize);
	if (node == nullptr)
	{
		String::IsAlphabet('a');
	}
	else
	{
		node->imageId = Random::Range(0, colors.GetSize() - 1);
		node->imageName = "Valid";
		root->usedArea += imageSize.x * imageSize.y;
	}
}

static void RemoveRect()
{
	auto node = root->FindUsedNode();
	if (node != nullptr)
	{
		root->usedArea -= Math::RoundToInt(node->rect.GetSize().x * node->rect.GetSize().y);
		node->ClearImage();
	}
	else
	{
		String::IsAlphabet('a');
	}
}

static void DoRectPacking(FusionRenderer2* renderer)
{
	renderer->SetBrush(Colors::Black);
	renderer->SetPen(FPen());

	renderer->PushChildCoordinateSpace(Vec2(0, 40));
	{
		// BG
		renderer->FillRect(Rect::FromSize(0, 0, root->rect.max.x, root->rect.max.y));

		root->ForEachRecursive([&](Ptr<FImageAtlas::BinaryNode> node)
			{
				if (node->imageId >= 0)
				{
					renderer->SetPen(FPen());
					renderer->SetBrush(colors[node->imageId % (int)colors.GetSize()]);

					renderer->FillRect(node->rect);
				}
				
				if (node->child[0] != nullptr || node->child[1] != nullptr)
				{
					renderer->SetPen(FPen(Colors::Red, 1.5f));
					renderer->StrokeRect(node->rect);
				}
				else
				{
					renderer->SetPen(FPen(Colors::White.WithAlpha(0.8f), 1));
					renderer->StrokeRect(Rect(node->rect.left + 1, node->rect.top + 1, node->rect.right - 1, node->rect.bottom - 1));
				}
			});

		renderer->SetPen(FPen(Colors::White));
		renderer->DrawSDFText(String::Format("Area: {}", root->usedArea), Vec2(100, 100));
	}
	renderer->PopChildCoordinateSpace();
}

TEST(FusionCore, Rendering)
{
	TEST_BEGIN_GUI;
	using namespace RenderingTests;

	f32 deltaTime = 0;
	clock_t previousTime = {};

	FusionApplication* app = FusionApplication::Get();
	FStyleManager* styleManager = app->GetStyleManager();

	FRootContext* rootContext = FusionApplication::Get()->GetRootContext();

	FStyleSet* rootStyle = CreateObject<FStyleSet>(rootContext, "RootStyleSet");
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
				.ScrollBarBrush(Color::RGBA(60, 60, 60))
				;
		}

		{
			auto windowCloseBtn = CreateObject<FCustomButtonStyle>(rootStyle, "WindowWindowClose");
			rootStyle->Add("Button.WindowClose", windowCloseBtn);

			windowCloseBtn->background = Colors::Clear;
			windowCloseBtn->hoveredBackground = Color::RGBA(161, 57, 57);
			windowCloseBtn->hoveredBackground = Color::RGBA(196, 43, 28);
			windowCloseBtn->pressedBackground = Color::RGBA(178, 43, 30);
			windowCloseBtn->borderColor = Colors::Clear;
			windowCloseBtn->borderWidth = 0.0f;
			windowCloseBtn->cornerRadius = Vec4();
			windowCloseBtn->contentMoveY = 0;

			auto windowControlBtn = CreateObject<FCustomButtonStyle>(rootStyle, "WindowControlButton");
			rootStyle->Add("Button.WindowControl", windowControlBtn);

			windowControlBtn->background = Colors::Clear;
			windowControlBtn->hoveredBackground = Color::RGBA(58, 58, 58);
			windowControlBtn->pressedBackground = Color::RGBA(48, 48, 48);
			windowControlBtn->borderColor = Colors::Clear;
			windowControlBtn->borderWidth = 0.0f;
			windowControlBtn->cornerRadius = Vec4();
			windowControlBtn->contentMoveY = 0;
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
	}

	PlatformWindow* mainWindow = PlatformApplication::Get()->GetMainWindow();

	FNativeContext* nativeContext = FNativeContext::Create(mainWindow, "TestWindow", rootContext);
	rootContext->AddChildContext(nativeContext);

	RenderingTestWidget* mainWidget;
	FAssignNewOwned(RenderingTestWidget, mainWidget, nativeContext);

	nativeContext->SetOwningWidget(mainWidget);

	mainWidget->OnAdd([]
	{
		AddRect();
	});
	mainWidget->OnRemove([]
	{
		clock_t prev = clock();

		RemoveRect();
		root->Defragment();

		CE_LOG(Info, All, "Remove Time: {} ms", ((f64)(clock() - prev) * 1000.0 / CLOCKS_PER_SEC));
	});
	mainWidget->OnDefragment([]
	{
		clock_t prev = clock();

		//root->Defragment();
		root->DefragmentSlow();

		CE_LOG(Info, All, "Defragment Time: {} ms", ((f64)(clock() - prev) * 1000.0 / CLOCKS_PER_SEC));
	});

	auto exposedTick = [&]
		{
			FusionApplication::Get()->SetExposed();

			FusionApplication::Get()->Tick();

			RendererSystem::Get().Render();
		};

	DelegateHandle handle = PlatformApplication::Get()->AddTickHandler(exposedTick);

	if (mainWidget->comboBox)
	{
		mainWidget->comboBox->ApplyStyle();
	}

	mainWindow->Show();

	int frameCounter = 0;

	root = new FImageAtlas::BinaryNode;
	root->rect = Rect(0, 0, 680, 460);
	
	for (int i = 0; i < rects.GetSize(); ++i)
	{
		auto node = root->Insert(rects[i]);
		if (node != nullptr)
		{
			node->imageId = i;
			node->imageName = "Valid";
			root->usedArea += rects[i].x * rects[i].y;
		}
		else
		{
			String::IsAlphabet('a');
		}
	}

	for (int i = 0; i < 92; ++i)
	{
		AddRect();
	}

	for (int i = 0; i < 92; ++i)
	{
		RemoveRect();
	}

	root->Defragment();
	//root->DefragmentSlow();

	while (!IsEngineRequestingExit())
	{
		auto curTime = clock();
		deltaTime = (f32)(curTime - previousTime) / CLOCKS_PER_SEC;

		FusionApplication::Get()->ResetExposed();

		// App & Input Tick
		PlatformApplication::Get()->Tick();
		InputManager::Get().Tick();

		RendererSystem::Get().Render();

		previousTime = curTime;
		frameCounter++;
	}

	PlatformApplication::Get()->RemoveTickHandler(handle);

	styleManager->DeregisterStyleSet(rootStyle->GetName());

	TEST_END_GUI;
}

