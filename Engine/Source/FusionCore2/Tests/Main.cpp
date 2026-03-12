
#include "FusionCore.h"

#if PAL_TRAIT_METAL_SUPPORTED
#   include "MetalRHI.h"
#endif

#if PAL_TRAIT_VULKAN_SUPPORTED
#   include "VulkanRHI.h"
#endif

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
#if PAL_TRAIT_METAL_SUPPORTED
    ModuleManager::Get().LoadModule("MetalRHI");
#endif
#if PAL_TRAIT_VULKAN_SUPPORTED
    ModuleManager::Get().LoadModule("VulkanRHI");
#endif
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

		constexpr f32 scaleFactor = 1.0f;

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
	
#if PAL_TRAIT_METAL_SUPPORTED
    RHI::gDynamicRHI = new Metal::MetalRHI();
#endif
#if PAL_TRAIT_VULKAN_SUPPORTED
    RHI::gDynamicRHI = new Vulkan::VulkanRHI();
#endif

	RHI::gDynamicRHI->Initialize();
	RHI::gDynamicRHI->PostInitialize();

	if (gui)
	{
		RHI::FrameSchedulerDescriptor desc{};
		desc.numFramesInFlight = 2;
		RHI::FrameScheduler::Create(desc);
	}

	RPI::RPISystem::Get().Initialize();

	if (gui)
	{
		FApplication* fApp = FApplication::GetOrCreate();

		FApplicationInitInfo initInfo = {};
		initInfo.services = {
			GetStaticClass<FEventService>(),
			GetStaticClass<FusionRenderService>(),
			GetStaticClass<FAnimationService>()
		};

		fApp->Initialize(initInfo);
	}

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

	if (gui)
	{
		FApplication* fApp = FApplication::Get();

		fApp->PreShutdown();

		fApp->Shutdown();

		fApp->BeginDestroy();

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
#if PAL_TRAIT_METAL_SUPPORTED
    ModuleManager::Get().UnloadModule("MetalRHI");
#endif
#if PAL_TRAIT_VULKAN_SUPPORTED
    ModuleManager::Get().UnloadModule("VulkanRHI");
#endif
	ModuleManager::Get().UnloadModule("CoreRHI");
	ModuleManager::Get().UnloadModule("CoreShader");
	ModuleManager::Get().UnloadModule("CoreMedia");
	ModuleManager::Get().UnloadModule("CoreInput");
	ModuleManager::Get().UnloadModule("CoreApplication");
	ModuleManager::Get().UnloadModule("Core");

	EXPECT_EQ(FApplication::Get(), nullptr);
}

TEST(FusionCore, Rendering)
{
	TEST_BEGIN_GUI;
	using namespace RenderingTests;

	f32 deltaTime = 0;
	clock_t previousTime = clock();

	PlatformWindow* mainWindow = PlatformApplication::Get()->GetMainWindow();

	FApplication* fApp = FApplication::Get();

	auto exposedTick = [&]
		{
			fApp->Tick(deltaTime, true);
		};

	DelegateHandle handle = PlatformApplication::Get()->AddTickHandler(exposedTick);

	mainWindow->Show();

	{
		Ref<FNativeSurface> nativeSurface = FNativeSurface::Create(mainWindow, "MainWindow", nullptr);
		Ref<TestWindow> testWindow;

		FAssignNewOwned(TestWindow, testWindow, nativeSurface.Get())
			.Name("TestWindow")
			.HAlign(HAlign::Fill)
			.VAlign(VAlign::Fill);

		nativeSurface->SetOwningWidget(testWindow);

		while (!IsEngineRequestingExit())
		{
			auto curTime = clock();
			deltaTime = (f32)(curTime - previousTime) / CLOCKS_PER_SEC;

			fApp->Tick(deltaTime, false);

			previousTime = curTime;
		}

		nativeSurface->BeginDestroy();
		nativeSurface = nullptr;
		testWindow = nullptr;
	}

	PlatformApplication::Get()->RemoveTickHandler(handle);

	TEST_END_GUI;
}

