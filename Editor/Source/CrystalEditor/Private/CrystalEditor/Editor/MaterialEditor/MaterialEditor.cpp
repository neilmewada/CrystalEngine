#include "CrystalEditor.h"

namespace CE::Editor
{
    HashMap<Uuid, WeakRef<MaterialEditor>> MaterialEditor::materialEditorsBySourceAssetUuid{};

    MaterialEditor::MaterialEditor()
    {

    }

    void MaterialEditor::Construct()
    {
        ZoneScoped;

        Super::Construct();

        Title("Material");

        minorDockspace->AddDockWindow(
            FAssignNew(EditorViewportTab, viewportTab)
        );

        viewportTab->GetDockspaceSplitView()->AddDockWindowSplit(FDockingHintPosition::Right,
            FAssignNew(MaterialDetailsTab, detailsTab)
        );

        viewportTab->GetDockspaceSplitView()->AddDockWindowSplit(FDockingHintPosition::Bottom,
            FAssignNew(AssetBrowser, assetBrowserTab)
        );

        toolBar->Content(
            EditorToolBar::NewImageButton("/Editor/Assets/Icons/Save")
            .OnClicked(FUNCTION_BINDING(this, SaveChanges))
        );

        detailsTab->SetOwnerEditor(this);

        viewportScene = CreateObject<CE::Scene>(this, "MaterialScene");
        viewportTab->GetViewport()->SetScene(viewportScene->GetRpiScene());

        EditorViewport* viewport = viewportTab->GetViewport();
        viewport->SetName("MaterialEditorViewport");

        auto assetManager = gEngine->GetAssetManager();

        Ref<TextureCube> skybox = assetManager->LoadAssetAtPath<TextureCube>("/Engine/Assets/Textures/HDRI/sample_day");

        Ref<CE::Shader> standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
        Ref<CE::Shader> skyboxShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/SkyboxCubeMap");

        viewportScene->SetSkyboxCubeMap(skybox.Get());

        CE::Material* aluminumMaterial = CreateObject<CE::Material>(viewportScene.Get(), "AluminumMaterial");
        aluminumMaterial->SetShader(standardShader.Get());
        {
            Ref<CE::Texture> albedoTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/albedo");
            Ref<CE::Texture> normalTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/normal");
            Ref<CE::Texture> metallicTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/metallic");
            Ref<CE::Texture> roughnessTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/roughness");

            aluminumMaterial->SetProperty("_AlbedoTex", albedoTex);
            aluminumMaterial->SetProperty("_NormalTex", normalTex);
            aluminumMaterial->SetProperty("_MetallicTex", metallicTex);
            aluminumMaterial->SetProperty("_RoughnessTex", roughnessTex);
            aluminumMaterial->ApplyProperties();
        }

        Ref<StaticMesh> sphereMesh = CreateObject<StaticMesh>(viewportScene.Get(), "Mat_SphereMesh");
        {
            RPI::ModelAsset* sphereModel = CreateObject<RPI::ModelAsset>(sphereMesh.Get(), "Mat_SphereModel");
            RPI::ModelLodAsset* sphereLodAsset = RPI::ModelLodAsset::CreateSphereAsset(sphereModel);
            sphereModel->AddModelLod(sphereLodAsset);

            sphereMesh->SetModelAsset(sphereModel);
        }

        Ref<StaticMeshActor> sphereActor = CreateObject<StaticMeshActor>(viewportScene.Get(), "Mat_SphereMesh");
        viewportScene->AddActor(sphereActor.Get());
        {
            sphereMeshComponent = sphereActor->GetMeshComponent();
            sphereMeshComponent->SetStaticMesh(sphereMesh);
            sphereMeshComponent->SetLocalPosition(Vec3(0, 0, 1.5f));
            sphereMeshComponent->SetLocalEulerAngles(Vec3(0, 0, 0));
            sphereMeshComponent->SetMaterial(aluminumMaterial, 0, 0);
        }

        Ref<CameraActor> camera = CreateObject<CameraActor>(viewportScene.Get(), "Mat_Camera");
        camera->GetCameraComponent()->SetLocalPosition(Vec3(0, 0, 0));
        viewportScene->AddActor(camera.Get());

        Ref<StaticMeshActor> skyboxActor = CreateObject<StaticMeshActor>(viewportScene.Get(), "Mat_SkyboxActor");
        viewportScene->AddActor(skyboxActor.Get());
        {
            StaticMeshComponent* skyboxMeshComponent = skyboxActor->GetMeshComponent();
            skyboxMeshComponent->SetStaticMesh(sphereMesh);

            skyboxMeshComponent->SetLocalPosition(Vec3(0, 0, 0));
            skyboxMeshComponent->SetLocalScale(Vec3(1, 1, 1) * 1000);

            {
                Ref<CE::Material> skyboxMaterial = CreateObject<CE::Material>(skyboxMeshComponent, "Mat_Material");
                skyboxMaterial->SetShader(skyboxShader.Get());
                skyboxMeshComponent->SetMaterial(skyboxMaterial.Get(), 0, 0);

                skyboxMaterial->SetProperty("_CubeMap", skybox.Get());
                skyboxMaterial->ApplyProperties();
            }
        }

        // For debugging only
        viewportScene->GetRpiScene()->SetName("MaterialScene");

		// Always need to add the scene and its corresponding viewport to the engine
        gEngine->AddScene(viewportScene.Get());
        gEditor->AddRenderViewport(viewportTab->GetViewport());
    }

    void MaterialEditor::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

    }

    void MaterialEditor::OnAssetUnloaded(Uuid bundleUuid)
    {
        if (targetMaterial == nullptr)
            return;

        Ref<Bundle> materialBundle = targetMaterial->GetBundle();

        if (!materialBundle || materialBundle->GetUuid() != bundleUuid)
            return;

        viewportTab->QueueDestroy();

        QueueDestroy();
        materialBundle = nullptr;
    }

    void MaterialEditor::OnAssetDeleted(const CE::Name& bundlePath)
    {
        if (targetMaterial == nullptr)
            return;

        AssetRegistry* assetRegistry = AssetRegistry::Get();
        Ref<EditorObjectUtility> objectUtility = EditorObjectUtility::Get();

        if (AssetData* assetData = assetRegistry->GetPrimaryAssetByPath(bundlePath))
        {
            Uuid bundleUuid = assetData->bundleUuid;
            if (Bundle::IsBundleLoaded(bundleUuid))
            {
                objectUtility->RemoveObjectReference(targetMaterial, [&](Ref<Object> object) -> bool
                {
                    if (!object)
                        return false;
                    if (Ref<Bundle> ownerBundle = object->GetBundle(); ownerBundle->GetUuid() == bundleUuid)
                    {
                        return true;
                    }
                    return false;
                });
            }
        }

        Ref<Bundle> materialBundle = targetMaterial->GetBundle();

        if (!materialBundle || materialBundle->GetBundlePath() != bundlePath)
            return;

        viewportTab->QueueDestroy();

        QueueDestroy();
        materialBundle = nullptr;
    }

    ClassType* MaterialEditor::GetTargetObjectType() const
    {
        return CE::Material::StaticClass();
    }

    bool MaterialEditor::CanEdit(Ref<Object> targetObject) const
    {
        return targetObject.IsValid() && targetObject->IsOfType<CE::Material>();
    }

    bool MaterialEditor::OpenEditor(Ref<Object> targetObject, Ref<Bundle> bundle)
    {
        if (!targetObject)
            return false;
        if (!targetObject->IsOfType<CE::Material>())
            return false;

		Super::OpenEditor(targetObject, bundle);

        Ref<CE::Material> material = (Ref<CE::Material>)targetObject;
        if (this->targetMaterial == material)
            return true;

        this->targetMaterial = material;
		this->bundle = bundle;
        SetMaterial(material);

        return true;
    }

    Ref<MaterialEditor> MaterialEditor::Open(const CE::Name& materialAssetPath)
    {
        AssetRegistry* registry = AssetRegistry::Get();
        AssetData* assetData = registry->GetPrimaryAssetByPath(materialAssetPath);
        if (!assetData)
            return nullptr;

        Ref<CE::Material> material = gEditor->GetAssetManager()->LoadAssetAtPath<CE::Material>(materialAssetPath);

        CrystalEditorWindow* crystalEditorWindow = CrystalEditorWindow::Get();

        if (auto existingEditor = materialEditorsBySourceAssetUuid[assetData->assetUuid].Lock())
        {
            crystalEditorWindow->SelectActiveEditor(existingEditor);
            existingEditor->SetMaterial(material);
            return existingEditor;
        }

        Ref<MaterialEditor> editor = CreateObject<MaterialEditor>(crystalEditorWindow, "MaterialEditor");
        materialEditorsBySourceAssetUuid[assetData->assetUuid] = editor;

        editor->SetMaterial(material);

        crystalEditorWindow->GetDockspace()->AddDockWindow(editor);
        crystalEditorWindow->SelectActiveEditor(editor);

        return editor;
    }

    Array<CE::Name> MaterialEditor::GetSelectedAssetPaths()
    {
        return assetBrowserTab->GetSelectedAssetPaths();
    }

    void MaterialEditor::BrowseToAsset(const CE::Name& path)
    {
        assetBrowserTab->BrowseToAsset(path);
    }

    void MaterialEditor::SaveChanges()
    {
        Super::SaveChanges();
    }

    void MaterialEditor::SetMaterial(Ref<CE::Material> material)
    {
        if (!material)
            return;

        String title = material->GetName().GetString();
        if (Ref<Bundle> bundle = material->GetBundle())
        {
            title = bundle->GetName().GetString();
        }

        sphereMeshComponent->SetMaterial(material.Get(), 0, 0);

        detailsTab->SetupEditor(material);

        Title(title);
    }
}

