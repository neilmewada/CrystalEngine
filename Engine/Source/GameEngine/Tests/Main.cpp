#include "Core.h"

#include "GameEngine.h"
#include "VulkanRHI.h"

#include <gtest/gtest.h>

using namespace CE;

#include "TestFeatureProcessor.h"

#include "GameEngine_Test.private.h"

using namespace CE;

#define TEST_BEGIN TestBegin(false)
#define TEST_END TestEnd(false)

#define TEST_BEGIN_WINDOWED TestBegin(true)
#define TEST_END_WINDOWED TestEnd(true)

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
	ModuleManager::Get().LoadModule("CrystalWidgets");
	ModuleManager::Get().LoadModule("Engine");
	ModuleManager::Get().LoadModule("GameEngine");

	PlatformApplication* app = PlatformApplication::Get();

	app->Initialize();

	if (gui)
	{
		PlatformWindowInfo windowInfo{};
		windowInfo.fullscreen = windowInfo.hidden = windowInfo.maximised = windowInfo.resizable = false;
		windowInfo.resizable = true;
		windowInfo.windowFlags = PlatformWindowFlags::DestroyOnClose;

		app->InitMainWindow("MainWindow", 1024, 768, windowInfo);

		InputManager::Get().Initialize(app);
	}

	gDynamicRHI = new Vulkan::VulkanRHI();

	gDynamicRHI->Initialize();
	gDynamicRHI->PostInitialize();

	RPISystem::Get().Initialize();

	Logger::Initialize();
}

static void TestEnd(bool gui)
{
	Logger::Shutdown();

	RPISystem::Get().Shutdown();

	gDynamicRHI->PreShutdown();
	gDynamicRHI->Shutdown();
	delete gDynamicRHI; gDynamicRHI = nullptr;

	PlatformApplication* app = PlatformApplication::Get();

	if (gui)
	{
		InputManager::Get().Shutdown(app);
	}

	app->PreShutdown();
	app->Shutdown();
	delete app;

	ModuleManager::Get().UnloadModule("GameEngine");
	ModuleManager::Get().UnloadModule("Engine");
	ModuleManager::Get().UnloadModule("CrystalWidgets");
	ModuleManager::Get().UnloadModule("CoreRPI");
	ModuleManager::Get().UnloadModule("VulkanRHI");
	ModuleManager::Get().UnloadModule("CoreRHI");
	ModuleManager::Get().UnloadModule("CoreShader");
	ModuleManager::Get().UnloadModule("CoreMedia");
	ModuleManager::Get().UnloadModule("CoreInput");
	ModuleManager::Get().UnloadModule("CoreApplication");
	ModuleManager::Get().UnloadModule("Core");
}



TEST(RPI, Scene)
{
	TEST_BEGIN_WINDOWED;
	CERegisterModuleTypes();

	auto app = PlatformApplication::Get();

	gEngine->PreInit();
	gEngine->Initialize();
	gEngine->PostInitialize();

	AssetManager* assetManager = AssetManager::Get();

	CE::Shader* standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
	CE::Shader* skyboxShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/SkyboxCubeMap");
	CE::Shader* iblConvolutionShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/CubeMap/IBLConvolution");
	TextureCube* skybox = assetManager->LoadAssetAtPath<TextureCube>("/Engine/Assets/Textures/HDRI/sample_night2");

	CE::Texture* albedoTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/albedo");
	CE::Texture* normalTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/normal");
	CE::Texture* metallicTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/metallic");
	CE::Texture* roughnessTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/roughness");

	RPI::RPISystemInitInfo rpiInitInfo{};
	rpiInitInfo.standardShader = standardShader->GetShaderCollection();
	rpiInitInfo.iblConvolutionShader = iblConvolutionShader->GetShaderCollection();

	RPISystem::Get().PostInitialize(rpiInitInfo);

	RendererSubsystem* rendererSubsystem = gEngine->GetSubsystem<RendererSubsystem>();
	SceneSubsystem* sceneSubsystem = gEngine->GetSubsystem<SceneSubsystem>();

	CE::Scene* scene = sceneSubsystem->GetMainScene();
	RPI::Scene* rpiScene = scene->GetRpiScene();

	scene->SetSkyboxCubeMap(skybox);

	TestFeatureProcessor1* fp1 = rpiScene->AddFeatureProcessor<TestFeatureProcessor1>();
	StaticMeshFeatureProcessor* meshFp = rpiScene->GetFeatureProcessor<StaticMeshFeatureProcessor>();
	
	{
		StaticMeshActor* meshActor = CreateObject<StaticMeshActor>(scene, "StaticMeshActor");
		scene->AddActor(meshActor);

		StaticMeshComponent* meshComponent = meshActor->GetMeshComponent();

		StaticMesh* cubeMesh = CreateObject<StaticMesh>(scene, "StaticMesh");
		{
			RPI::ModelAsset* cubeModel = CreateObject<ModelAsset>(cubeMesh, "CubeModel");
			RPI::ModelLodAsset* cubeLodAsset = RPI::ModelLodAsset::CreateCubeAsset(cubeModel);
			cubeModel->AddModelLod(cubeLodAsset);

			cubeMesh->SetModelAsset(cubeModel);
		}
		meshComponent->SetStaticMesh(cubeMesh);

		meshComponent->SetLocalPosition(Vec3(0, 0, 10));
		meshComponent->SetLocalEulerAngles(Vec3(0, 0, 0));

		StaticMesh* sphereMesh = CreateObject<StaticMesh>(scene, "SphereMesh");
		{
			RPI::ModelAsset* sphereModel = CreateObject<ModelAsset>(sphereMesh, "SphereModel");
			RPI::ModelLodAsset* sphereLodAsset = RPI::ModelLodAsset::CreateSphereAsset(sphereModel);
			sphereModel->AddModelLod(sphereLodAsset);

			sphereMesh->SetModelAsset(sphereModel);
		}
		meshComponent->SetStaticMesh(sphereMesh);

		StaticMeshActor* skyboxActor = CreateObject<StaticMeshActor>(scene, "SkyboxActor");
		scene->AddActor(skyboxActor);

		StaticMeshComponent* skyboxMeshComponent = skyboxActor->GetMeshComponent();
		skyboxMeshComponent->SetStaticMesh(sphereMesh);

		skyboxMeshComponent->SetLocalPosition(Vec3(0, 0, 0));
		skyboxMeshComponent->SetLocalScale(Vec3(1, 1, 1) * 1000);
		
		{
			CE::Material* material = CreateObject<CE::Material>(meshComponent, "Material");
			material->SetShader(standardShader);
			meshComponent->SetMaterial(material, 0, 0);

			material->SetProperty("_AlbedoTex", albedoTex);
			material->SetProperty("_NormalTex", normalTex);
			material->SetProperty("_MetallicTex", metallicTex);
			material->SetProperty("_RoughnessTex", roughnessTex);
			material->ApplyProperties();
		}

		{
			CE::Material* skyboxMaterial = CreateObject<CE::Material>(skyboxMeshComponent, "Material");
			skyboxMaterial->SetShader(skyboxShader);
			skyboxMeshComponent->SetMaterial(skyboxMaterial, 0, 0);

			skyboxMaterial->SetProperty("_CubeMap", skybox);
			skyboxMaterial->ApplyProperties();
		}

		CameraComponent* cameraComponent = CreateObject<CameraComponent>(meshComponent, "Camera");
		cameraComponent->SetLocalPosition(Vec3(0, 0, -10));
		cameraComponent->SetFarPlane(500);
		meshComponent->SetupAttachment(cameraComponent);

		clock_t lastTime = clock();
		constexpr u32 LoopCount = 256;
		u32 frameCounter = 0;

		f32 rotation = 0.0f;
		f32 camPos = cameraComponent->GetLocalPosition().z;

		while (!IsEngineRequestingExit())
		{
			auto curTime = clock();
			f32 deltaTime = ((f32)(curTime - lastTime)) / CLOCKS_PER_SEC;

			if (InputManager::IsKeyDown(KeyCode::Escape))
			{
				RequestEngineExit("USER_EXIT");
			}

			if (InputManager::IsKeyDown(KeyCode::A))
			{
				meshComponent->SetStaticMesh(cubeMesh);
			}
			else if (InputManager::IsKeyDown(KeyCode::S))
			{
				meshComponent->SetStaticMesh(sphereMesh);
			}

			app->Tick();
			InputManager::Get().Tick();

			rotation += deltaTime * 30;
			if (rotation > 360)
				rotation -= 360;
			camPos += deltaTime * 2;
			if (camPos > 0)
				camPos = -10;

			meshComponent->SetLocalEulerAngles(Vec3(0, 0, rotation));
			cameraComponent->SetLocalPosition(Vec3(0, 0, -2));
			//cameraComponent->SetLocalEulerAngles(Vec3(0, 0, rotation));
			
			gEngine->Tick(deltaTime);

			lastTime = curTime;

			frameCounter++;
		}

	}

	gEngine->PreShutdown();
	gEngine->Shutdown();

	CEDeregisterModuleTypes();
	TEST_END_WINDOWED;
}

