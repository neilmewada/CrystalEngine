#include "GameLauncher.h"


void GameLoop::SetupTestScene()
{
	AssetManager* assetManager = AssetManager::Get();

	Ref<TextureCube> skybox = assetManager->LoadAssetAtPath<TextureCube>("/Engine/Assets/Textures/HDRI/sample_night");
	Ref<CE::Shader> standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
	Ref<CE::Shader> skyboxShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/SkyboxCubeMap");

	CE::Scene* scene = CreateObject<CE::Scene>(gEngine, "TestScene");

	GameViewportSubsystem* viewportSubsystem = gEngine->GetSubsystem<GameViewportSubsystem>();
	gameWindow = viewportSubsystem->GetGameWindow();

	scene->SetSkyboxCubeMap(skybox.Get());

	viewportSubsystem->SetScene(scene);
	gEngine->LoadScene(scene);

	{
		Actor* sandboxActor = CreateObject<Actor>(scene, "SandboxActor");

		SandboxComponent* component = CreateObject<SandboxComponent>(sandboxActor, "SandboxComponent");
		sandboxActor->AttachComponent(component);

		scene->AddActor(sandboxActor);

		component->SetupScene();
	}
}

