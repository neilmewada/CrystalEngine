
#include "Fusion.h"

#if PAL_TRAIT_VULKAN_SUPPORTED
#   include "VulkanRHI.h"
#endif
#if PAL_TRAIT_METAL_SUPPORTED
#   include "MetalRHI.h"
#endif

#include "FusionTest.h"

#include <gtest/gtest.h>

#include "Fusion_Test.private.h"

static CE::JobManager* gJobManager = nullptr;
static CE::JobContext* gJobContext = nullptr;

using namespace CE;
using namespace WidgetTests;

static int windowWidth = 0;
static int windowHeight = 0;

TEST(Fusion, MainTest)
{
	gProjectPath = PlatformDirectories::GetLaunchDir();
	gProjectName = MODULE_NAME;

	ModuleManager::Get().LoadModule("Core");
	ModuleManager::Get().LoadModule("CoreApplication");
	ModuleManager::Get().LoadModule("CoreInput");
	ModuleManager::Get().LoadModule("CoreMedia");
	ModuleManager::Get().LoadModule("CoreShader");
	ModuleManager::Get().LoadModule("CoreRHI");
#if PAL_TRAIT_VULKAN_SUPPORTED
    ModuleManager::Get().LoadModule("VulkanRHI");
#endif
#if PAL_TRAIT_METAL_SUPPORTED
    ModuleManager::Get().LoadModule("MetalRHI");
#endif
	ModuleManager::Get().LoadModule("CoreRPI");
	ModuleManager::Get().LoadModule("FusionCore");
	ModuleManager::Get().LoadModule("Fusion");
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
		u32 w = 600, h = 500;
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

#if PAL_TRAIT_VULKAN_SUPPORTED
	RHI::gDynamicRHI = new Vulkan::VulkanRHI();
#elif PAL_TRAIT_METAL_SUPPORTED
    RHI::gDynamicRHI = new Metal::MetalRHI();
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
	ModuleManager::Get().UnloadModule("Fusion");
#if PAL_TRAIT_VULKAN_SUPPORTED
    ModuleManager::Get().UnloadModule("VulkanRHI");
#endif
#if PAL_TRAIT_METAL_SUPPORTED
    ModuleManager::Get().UnloadModule("MetalRHI");
#endif
	ModuleManager::Get().UnloadModule("CoreRHI");
	ModuleManager::Get().UnloadModule("CoreShader");
	ModuleManager::Get().UnloadModule("CoreMedia");
	ModuleManager::Get().UnloadModule("CoreInput");
	ModuleManager::Get().UnloadModule("CoreApplication");
	ModuleManager::Get().UnloadModule("Core");
}

TEST(Fusion, TreeView)
{
	TEST_BEGIN_GUI;
	using namespace WidgetTests;

	FusionStandaloneApplication app{};
	CERegisterModuleTypes();

	PlatformWindowInfo windowInfo{};
	windowInfo.fullscreen = windowInfo.hidden = windowInfo.maximised = windowInfo.resizable = false;
	windowInfo.resizable = true;
	windowInfo.hidden = true;
	windowInfo.windowFlags = PlatformWindowFlags::DestroyOnClose;

#if PLATFORM_MAC
	u32 w = 600, h = 500;
#elif PLATFORM_LINUX
	u32 w = 1024, h = 768;
#elif PLATFORM_WINDOWS
	u32 w = 1400, h = 1000;
#endif

	app.Launch<FusionTestWindow>(w, h, windowInfo);

	CEDeregisterModuleTypes();
}

