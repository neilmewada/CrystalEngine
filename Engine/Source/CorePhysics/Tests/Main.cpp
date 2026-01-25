
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

	Ref<Object> transient = GetTransient("CorePhysics");

	Ref<PhysicsScene> scene = CreateObject<PhysicsScene>(transient.Get(), "PhysicsScene");

	BoxShapeSettings boxShapeSettings{ Vec3(10, 0.5f, 10), scene };
	Ref<BoxShape> groundShape = BoxShape::Create(boxShapeSettings, scene);

	PhysicsBodyInitInfo groundInit{};
	groundInit.objectName = "Ground";
	groundInit.ownerScene = scene;
	groundInit.layer = BuiltinPhysicsLayer::Default;
	groundInit.motionType = PhysicsMotionType::Static;
	groundInit.position = Vec3(0, 0, 0);
	groundInit.shape = groundShape;
	Ref<PhysicsBody> ground = scene->AddBody(groundInit);

	BoxShapeSettings cubeShapeSettings{ Vec3(1, 1, 1), scene };
	Ref<BoxShape> cubeShape = BoxShape::Create(cubeShapeSettings, scene);

	PhysicsBodyInitInfo cubeInit{};
	cubeInit.objectName = "Cube";
	cubeInit.ownerScene = scene;
	cubeInit.layer = BuiltinPhysicsLayer::Default;
	cubeInit.motionType = PhysicsMotionType::Dynamic;
	cubeInit.position = Vec3(0, 20, 0);
	cubeInit.shape = cubeShape;
	Ref<PhysicsBody> cube = scene->AddBody(cubeInit);

	int counter = 0;
	static constexpr int LoopCount = 1'000;

	scene->SetSimulationEnabled(true);

	while (counter < LoopCount)
	{
		Vec3 pos = cube->GetPosition();

		PhysicsSystem::Get().Tick(1 / 60.0f);

		counter++;
	}

	scene->RemoveBody(cube);
	scene->RemoveBody(ground);

	scene->BeginDestroy();
	scene = nullptr;

	TEST_END;
}

