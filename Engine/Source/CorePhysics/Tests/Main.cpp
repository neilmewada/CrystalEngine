
#include "CorePhysics.h"

#include <gtest/gtest.h>

#include "CorePhysics_Test.private.h"

#define TEST_BEGIN TestBegin()
#define TEST_END TestEnd()

using namespace CE;

static void TestBegin()
{
	ModuleManager::Get().LoadModule("Core");
	ModuleManager::Get().LoadModule("CorePhysics");
	CERegisterModuleTypes();

	Logger::Initialize();

	PhysicsSystemInitInfo initInfo{};

	PhysicsSystem::Get().Initialize(initInfo);
}

static void TestEnd()
{
	PhysicsSystem::Get().Shutdown();

	Logger::Shutdown();

	CEDeregisterModuleTypes();
	ModuleManager::Get().UnloadModule("CorePhysics");
	ModuleManager::Get().UnloadModule("Core");
}

TEST(CorePhysics, MainTest)
{
	TEST_BEGIN;

	int counter = 0;

	while (counter < 1000)
	{
		PhysicsSystem::Get().Tick(1 / 60.0f);

		counter++;
	}

	TEST_END;
}
