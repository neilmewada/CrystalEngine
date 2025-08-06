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

        Ref<TextureCube> skybox = assetManager->LoadAssetAtPath<TextureCube>("/Engine/Assets/Textures/HDRI/sample_night");
        Ref<CE::Shader> standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
        Ref<CE::Shader> skyboxShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/SkyboxCubeMap");

		CE::Scene* scene = GetScene();

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
		camera->GetCameraComponent()->SetLocalPosition(Vec3(0, 0, 2));
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

			skyboxMaterial->SetProperty("_CubeMap", skybox.Get());
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

		// - Ground -

		StaticMeshActor* groundMesh = CreateObject<StaticMeshActor>(scene, "GroundPlane");
		scene->AddActor(groundMesh);
		{
			StaticMeshComponent* meshComponent = groundMesh->GetMeshComponent();
			meshComponent->SetStaticMesh(cubeMesh);
			meshComponent->SetLocalPosition(Vec3(0, -0.75f, 5));
			meshComponent->SetLocalScale(Vec3(5, 0.05f, 5));
			meshComponent->SetMaterial(woodMaterial, 0, 0);
		}

		// - Sun -

		sunActor = CreateObject<DirectionalLight>(scene, "Sun");
		scene->AddActor(sunActor);
		{
			sunLight = sunActor->GetDirectionalLightComponent();

			sunLight->SetLocalPosition(Vec3(0, 0, 0));
			sunLight->SetLocalEulerAngles(Vec3(30, 0, 0));
			sunLight->SetIntensity(20.0f);
			sunLight->SetLightColor(Colors::White);
		}
    }

	void SandboxComponent::Tick(f32 delta)
	{
		Super::Tick(delta);

		elapsedTime += delta;

		cameraComponent->SetLocalEulerAngles(Vec3(0, 0, elapsedTime * 15));
	}

} // namespace CE

