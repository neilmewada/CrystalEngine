#include "Sandbox.h"

namespace CE
{

    SandboxComponent::SandboxComponent()
    {
		canTick = true;
    }

    SandboxComponent::~SandboxComponent()
    {
        
    }

    void SandboxComponent::SetupScene()
    {
        AssetManager* assetManager = AssetManager::Get();

        Ref<TextureCube> skybox = assetManager->LoadAssetAtPath<TextureCube>("/Engine/Assets/Textures/HDRI/sample_day");
        Ref<CE::Shader> standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
        Ref<CE::Shader> skyboxShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/SkyboxCubeMap");

		CE::Scene* scene = GetScene().Get();
		scene->SetSkyboxCubeMap(skybox.Get());

		Ref<CE::StaticMesh> sponzaMesh = assetManager->LoadAssetAtPath<StaticMesh>("/Engine/Assets/Sponza/NewSponza_Main_Yup_003");
		for (int i = 0; i < sponzaMesh->GetBuiltinMaterialCount(); i++)
		{
			sponzaMesh->GetBuiltinMaterial(i)->SetShader(standardShader);
		}

		// - Textures & Materials -

		Ref<CE::Texture> aluminiumAlbedoTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/albedo");
		Ref<CE::Texture> aluminiumNormalTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/normal");
		Ref<CE::Texture> aluminiumMetallicTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/metallic");
		Ref<CE::Texture> aluminiumRoughnessTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/roughness");

		Ref<CE::Texture> plasticAlbedoTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Plastic/albedo");
		Ref<CE::Texture> plasticNormalTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Plastic/normal");
		Ref<CE::Texture> plasticMetallicTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Plastic/metallic");
		Ref<CE::Texture> plasticRoughnessTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Plastic/roughness");

		Ref<CE::Texture> woodAlbedoTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/WoodFloor/albedo");
		Ref<CE::Texture> woodNormalTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/WoodFloor/normal");
		Ref<CE::Texture> woodMetallicTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/WoodFloor/metallic");
		Ref<CE::Texture> woodRoughnessTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/WoodFloor/roughness");

		CE::Material* aluminiumMaterial = CreateObject<CE::Material>(scene, "Material");
		aluminiumMaterial->SetShader(standardShader.Get());
		{
			aluminiumMaterial->SetProperty("_AlbedoTex", aluminiumAlbedoTex);
			aluminiumMaterial->SetProperty("_NormalTex", aluminiumNormalTex);
			aluminiumMaterial->SetProperty("_MetallicTex", aluminiumMetallicTex);
			aluminiumMaterial->SetProperty("_RoughnessTex", aluminiumRoughnessTex);
			aluminiumMaterial->ApplyProperties();
		}

		CE::Material* plasticMaterial = CreateObject<CE::Material>(scene, "PlasticMaterial");
		plasticMaterial->SetShader(standardShader.Get());
		{
			plasticMaterial->SetProperty("_AlbedoTex", plasticAlbedoTex);
			plasticMaterial->SetProperty("_NormalTex", plasticNormalTex);
			plasticMaterial->SetProperty("_MetallicTex", plasticMetallicTex);
			plasticMaterial->SetProperty("_RoughnessTex", plasticRoughnessTex);
			plasticMaterial->ApplyProperties();
		}

		CE::Material* woodMaterial = CreateObject<CE::Material>(scene, "WoodMaterial");
		woodMaterial->SetShader(standardShader.Get());
		{
			woodMaterial->SetProperty("_AlbedoTex", woodAlbedoTex);
			woodMaterial->SetProperty("_NormalTex", woodNormalTex);
			woodMaterial->SetProperty("_MetallicTex", woodMetallicTex);
			woodMaterial->SetProperty("_RoughnessTex", woodRoughnessTex);
			woodMaterial->ApplyProperties();
		}

		// - Meshes -

		StaticMesh* sphereMesh = CreateObject<StaticMesh>(scene, "SphereMesh");
		{
			RPI::ModelAsset* sphereModel = CreateObject<RPI::ModelAsset>(sphereMesh, "SphereModel");
			RPI::ModelLodAsset* sphereLodAsset = RPI::ModelLodAsset::CreateSphereAsset(sphereModel);
			sphereModel->AddModelLod(sphereLodAsset);

			sphereMesh->SetModelAsset(sphereModel);
		}

		StaticMesh* cubeMesh = CreateObject<StaticMesh>(scene, "CubeMesh");
		{
			RPI::ModelAsset* cubeModel = CreateObject<RPI::ModelAsset>(cubeMesh, "CubeModel");
			RPI::ModelLodAsset* cubeLodAsset = RPI::ModelLodAsset::CreateCubeAsset(cubeModel);
			cubeModel->AddModelLod(cubeLodAsset);

			cubeMesh->SetModelAsset(cubeModel);
		}

		// - Camera -

		CameraActor* camera = CreateObject<CameraActor>(scene, "Camera");
		camera->GetCameraComponent()->SetLocalPosition(Vec3(0, -2, -2));
		scene->AddActor(camera);

		cameraComponent = camera->GetCameraComponent();
		cameraComponent->SetFieldOfView(60);

		// - Skybox -

		StaticMeshActor* skyboxActor = CreateObject<StaticMeshActor>(scene, "SkyboxActor");
		scene->AddActor(skyboxActor);
		{
			StaticMeshComponent* skyboxMeshComponent = skyboxActor->GetMeshComponent();
			skyboxMeshComponent->SetStaticMesh(sphereMesh);

			skyboxMeshComponent->SetLocalPosition(Vec3(0, 0, 0));
			skyboxMeshComponent->SetLocalScale(Vec3(1, 1, 1) * 1000);

			CE::Material* skyboxMaterial = CreateObject<CE::Material>(skyboxMeshComponent, "Material");
			skyboxMaterial->SetShader(skyboxShader.Get());
			skyboxMeshComponent->SetMaterial(skyboxMaterial, 0, 0);

			skyboxMaterial->SetProperty("_CubeMap", MaterialTextureValue(skybox.Get()));
			skyboxMaterial->ApplyProperties();
		}

		// - Mesh 1 -

		StaticMeshActor* mesh1 = CreateObject<StaticMeshActor>(scene, "Mesh_1");
		scene->AddActor(mesh1);
		{
			StaticMeshComponent* meshComponent = mesh1->GetMeshComponent();
			meshComponent->SetStaticMesh(sphereMesh);
			meshComponent->SetLocalPosition(Vec3(0, 0, 5));
			meshComponent->SetMaterial(aluminiumMaterial, 0, 0);
		}

		// - Sponza -
		StaticMeshActor* sponzaActor = CreateObject<StaticMeshActor>(scene, "SponzaMesh");
		scene->AddActor(sponzaActor);
        {
			StaticMeshComponent* meshComponent = sponzaActor->GetMeshComponent();
			meshComponent->SetName("DebugMeshComponent");
			meshComponent->SetStaticMesh(sponzaMesh);
			meshComponent->SetLocalPosition(Vec3(0, 0, 0));
			meshComponent->SetLocalEulerAngles(Vec3(-90, 0, 0));
			meshComponent->SetLocalScale(Vec3(1, 1, 1) * 0.01f);
        }

		// - Sun -

		sunActor = CreateObject<DirectionalLight>(scene, "Sun");
		scene->AddActor(sunActor);
		{
			sunLight = sunActor->GetDirectionalLightComponent();

			sunLight->SetLocalPosition(Vec3(0, 0, 0));
			sunLight->SetLocalEulerAngles(Vec3(70, 10, 0));
			sunLight->SetIntensity(50.0f);
			sunLight->SetLightColor(Colors::White);
			sunLight->SetShadowDistance(25);
		}
    }

	void SandboxComponent::Tick(f32 delta)
	{
		Super::Tick(delta);

		elapsedTime += delta;

		float forwardMotor = 0;
		float rightwardMotor = 0;
		float upwardMotor = 0;
		float eulerMotor = 0;

		if (InputManager::IsKeyHeld(KeyCode::W))
		{
			forwardMotor += 1;
		}
		if (InputManager::IsKeyHeld(KeyCode::S))
		{
			forwardMotor -= 1;
		}
		if (InputManager::IsKeyHeld(KeyCode::A))
		{
			rightwardMotor -= 1;
		}
		if (InputManager::IsKeyHeld(KeyCode::D))
		{
			rightwardMotor += 1;
		}
		if (InputManager::IsKeyHeld(KeyCode::Q))
		{
			if (InputManager::IsKeyHeld(KeyCode::LShift))
				eulerMotor -= 1;
			else
				upwardMotor += 1;
		}
		if (InputManager::IsKeyHeld(KeyCode::E))
		{
			if (InputManager::IsKeyHeld(KeyCode::LShift))
				eulerMotor += 1;
			else
				upwardMotor -= 1;
		}

		Vec3 pos = cameraComponent->GetPosition();
		pos += cameraComponent->GetForwardVector() * forwardMotor * cameraMoveSpeed * delta;
		pos += cameraComponent->GetRightwardVector() * rightwardMotor * cameraMoveSpeed * delta;
		pos += cameraComponent->GetUpwardVector() * upwardMotor * cameraMoveSpeed * delta;
		cameraComponent->SetPosition(pos);

		Vec3 euler = cameraComponent->GetLocalEulerAngles();
		euler.y += eulerMotor * cameraRotateSpeed * delta;
		cameraComponent->SetLocalEulerAngles(euler);
	}

} // namespace CE

