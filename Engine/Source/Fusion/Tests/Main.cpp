
#include "Fusion.h"
#include "VulkanRHI.h"

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

