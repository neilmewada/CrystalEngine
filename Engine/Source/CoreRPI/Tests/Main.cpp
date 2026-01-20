#include "Core.h"

#include "CoreRPI.h"
#include "VulkanRHI.h"

#include <gtest/gtest.h>

#include "CoreRPI_Test.private.h"

#include "TestFeatureProcessor.h"

using namespace CE;

#define TEST_BEGIN TestBegin()
#define TEST_END TestEnd()

static void TestBegin()
{
	gProjectPath = PlatformDirectories::GetLaunchDir();
	gProjectName = MODULE_NAME;

	ModuleManager::Get().LoadModule("Core");
	ModuleManager::Get().LoadModule("CoreApplication");
	ModuleManager::Get().LoadModule("CoreMedia");
	ModuleManager::Get().LoadModule("CoreShader");
	ModuleManager::Get().LoadModule("CoreRHI");
	ModuleManager::Get().LoadModule("VulkanRHI");
	ModuleManager::Get().LoadModule("CoreRPI");

	PlatformApplication* app = PlatformApplication::Get();
	
	app->Initialize();

	gDynamicRHI = new Vulkan::VulkanRHI();

	gDynamicRHI->Initialize();
	gDynamicRHI->PostInitialize();

	RPISystem::Get().Initialize();
}

static void TestEnd()
{
	RPISystem::Get().Shutdown();

	gDynamicRHI->PreShutdown();
	gDynamicRHI->Shutdown();
	delete gDynamicRHI; gDynamicRHI = nullptr;

	PlatformApplication* app = PlatformApplication::Get();
	app->PreShutdown();
	app->Shutdown();
	delete app;

	ModuleManager::Get().UnloadModule("CoreRPI");
	ModuleManager::Get().UnloadModule("VulkanRHI");
	ModuleManager::Get().UnloadModule("CoreRHI");
	ModuleManager::Get().UnloadModule("CoreShader");
	ModuleManager::Get().UnloadModule("CoreMedia");
	ModuleManager::Get().UnloadModule("CoreApplication");
	ModuleManager::Get().UnloadModule("Core");
}


TEST(RPI, Scene)
{
	TEST_BEGIN;
	CERegisterModuleTypes();

	Scene* scene = new Scene();
	scene->AddFeatureProcessor<TestFeatureProcessor1>();

	EXPECT_EQ(scene->featureProcessors.GetSize(), 1);
	EXPECT_TRUE(scene->featureProcessors[0]->IsOfType<TestFeatureProcessor1>());

	delete scene;

	CEDeregisterModuleTypes();
	TEST_END;
}

