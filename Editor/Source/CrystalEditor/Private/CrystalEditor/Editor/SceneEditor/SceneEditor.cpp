#include "CrystalEditor.h"

namespace CE::Editor
{
    ClassType* SceneEditor::GetTargetObjectType() const
    {
        return CE::Scene::StaticClass();
    }

    bool SceneEditor::CanEdit(Ref<Object> targetObject) const
    {
        return targetObject.IsValid() && targetObject->IsOfType<CE::Scene>();
    }

    Array<CE::Name> SceneEditor::GetSelectedAssetPaths()
    {
        return assetBrowser->GetSelectedAssetPaths();
    }

    void SceneEditor::BrowseToAsset(const CE::Name& path)
    {
        assetBrowser->BrowseToAsset(path);
    }

    SceneEditor::SceneEditor()
    {

    }

    void SceneEditor::LoadSandboxScene()
    {
        CE::Scene* scene = CreateObject<CE::Scene>(this, "SandboxScene");
        sandboxScene = scene;

        EditorViewport* viewport = viewportTab->GetViewport();
        viewport->SetScene(scene->GetRpiScene());

        auto assetManager = gEngine->GetAssetManager();

        Ref<TextureCube> skybox = assetManager->LoadAssetAtPath<TextureCube>("/Engine/Assets/Textures/HDRI/sample_night");
        Ref<CE::Shader> standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
        Ref<CE::Shader> skyboxShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/SkyboxCubeMap");

    	Ref<CE::Texture> albedoTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/albedo");
        Ref<CE::Texture> normalTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/normal");
        Ref<CE::Texture> metallicTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/metallic");
        Ref<CE::Texture> roughnessTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Aluminum/roughness");

    	Ref<CE::Texture> plasticAlbedoTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Plastic/albedo");
        Ref<CE::Texture> plasticNormalTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Plastic/normal");
        Ref<CE::Texture> plasticMetallicTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Plastic/metallic");
        Ref<CE::Texture> plasticRoughnessTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/Plastic/roughness");

    	Ref<CE::Texture> woodAlbedoTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/WoodFloor/albedo");
        Ref<CE::Texture> woodNormalTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/WoodFloor/normal");
        Ref<CE::Texture> woodMetallicTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/WoodFloor/metallic");
        Ref<CE::Texture> woodRoughnessTex = assetManager->LoadAssetAtPath<CE::Texture>("/Engine/Assets/Textures/WoodFloor/roughness");

        Ref<CE::Material> customMaterial = assetManager->LoadAssetAtPath<CE::Material>("/Game/Assets/Materials/NewMaterial");

        scene->SetSkyboxCubeMap(skybox.Get());

        {
            CE::Material* aluminumMaterial = CreateObject<CE::Material>(scene, "AluminumMaterial");
            aluminumMaterial->SetShader(standardShader.Get());

            aluminumMaterial->SetProperty("_AlbedoTex", albedoTex);
            aluminumMaterial->SetProperty("_NormalTex", normalTex);
            aluminumMaterial->SetProperty("_MetallicTex", metallicTex);
            aluminumMaterial->SetProperty("_RoughnessTex", roughnessTex);
            aluminumMaterial->ApplyProperties();

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

            Ref<StaticMesh> chairMesh = assetManager->LoadAssetAtPath<StaticMesh>("/Engine/Assets/Sandbox/chair");
            StaticMeshActor* chairActor = CreateObject<StaticMeshActor>(scene, "ChairActor");
            scene->AddActor(chairActor);
            {
                StaticMeshComponent* meshComponent = chairActor->GetMeshComponent();
                meshComponent->SetStaticMesh(chairMesh);
                meshComponent->SetLocalPosition(Vec3(0, 0, 5));
                meshComponent->SetLocalEulerAngles(Vec3(-90, -180, 0));
                meshComponent->SetLocalScale(Vec3(1, 1, 1) * 0.05f);

                meshComponent->SetMaterial(plasticMaterial, 0, 0);
                meshComponent->SetMaterial(plasticMaterial, 0, 1);
            }

            Ref<StaticMesh> carMesh = assetManager->LoadAssetAtPath<StaticMesh>("/Engine/Assets/Sandbox/car");
            StaticMeshActor* carActor = CreateObject<StaticMeshActor>(scene, "CarActor");
            scene->AddActor(carActor);
            {
                StaticMeshComponent* meshComponent = carActor->GetMeshComponent();
                meshComponent->SetStaticMesh(carMesh);
                meshComponent->SetLocalPosition(Vec3(0, 0, 5));
                meshComponent->SetLocalEulerAngles(Vec3(0, -90, 0));
                meshComponent->SetLocalScale(Vec3(1, 1, 1));

                auto modelAsset = carMesh->GetModelAsset();
                auto lodAsset = modelAsset->GetModelLod(0);

                for (int i = 0; i < meshComponent->GetLodSubMeshCount(0); ++i)
                {
                    const RPI::ModelLodSubMesh& subMesh = lodAsset->GetSubMesh(i);
                    Ref<CE::Material> builtinMaterial = carMesh->GetBuiltinMaterial(subMesh.materialIndex);
                    //builtinMaterial->SetShader(standardShader);

                    //builtinMaterial->SetProperty("_AlbedoTex", plasticAlbedoTex);
                    //builtinMaterial->SetProperty("_NormalTex", plasticNormalTex);
                    //builtinMaterial->SetProperty("_MetallicTex", plasticMetallicTex);
                    //builtinMaterial->SetProperty("_RoughnessTex", plasticRoughnessTex);
                    //builtinMaterial->ApplyProperties();

                    //meshComponent->SetMaterial(aluminumMaterial, 0, i);
                    meshComponent->SetMaterial(builtinMaterial.Get(), 0, subMesh.materialIndex);
                }
            }

            StaticMeshActor* sphereActor = CreateObject<StaticMeshActor>(scene, "SphereMesh");
            scene->AddActor(sphereActor);
	        {
		        StaticMeshComponent* meshComponent = sphereActor->GetMeshComponent();
            	meshComponent->SetStaticMesh(sphereMesh);
            	meshComponent->SetLocalPosition(Vec3(3, 0, 5));
            	meshComponent->SetLocalEulerAngles(Vec3(0, 0, 0));
                meshComponent->SetMaterial(aluminumMaterial, 0, 0);
	        }

            //if (customMaterial)
            {
                StaticMeshActor* sphereActor2 = CreateObject<StaticMeshActor>(scene, "SphereMesh_2");
                scene->AddActor(sphereActor2);
                {
                    StaticMeshComponent* meshComponent = sphereActor2->GetMeshComponent();
                    meshComponent->SetStaticMesh(sphereMesh);
                    meshComponent->SetLocalPosition(Vec3(3, 1, 5));
                    meshComponent->SetLocalEulerAngles(Vec3(0, 0, 0));
                    meshComponent->SetMaterial(customMaterial.Get(), 0, 0);
                }
            }

            StaticMeshActor* groundActor = CreateObject<StaticMeshActor>(scene, "Ground");
            scene->AddActor(groundActor);
            {
                StaticMeshComponent* meshComponent = groundActor->GetMeshComponent();
                meshComponent->SetStaticMesh(cubeMesh);
                meshComponent->SetLocalPosition(Vec3(0, -1, 5));
                meshComponent->SetLocalScale(Vec3(10, 0.1f, 10));
                meshComponent->SetMaterial(woodMaterial, 0, 0);
            }

            DirectionalLight* lightActor = CreateObject<DirectionalLight>(scene, "Sun");
            scene->AddActor(lightActor);
            {
                DirectionalLightComponent* directionalLight = lightActor->GetDirectionalLightComponent();
                directionalLight->SetLocalEulerAngles(Vec3(30, 0, 0));
                directionalLight->SetIntensity(10);
                directionalLight->SetLightColor(Color::White());
            }

            CameraActor* camera = CreateObject<CameraActor>(scene, "Camera");
            camera->GetCameraComponent()->SetLocalPosition(Vec3(0, 0, 0));
            scene->AddActor(camera);

            CameraComponent* cameraComponent = camera->GetCameraComponent();
            cameraComponent->SetFieldOfView(60);

            StaticMeshActor* skyboxActor = CreateObject<StaticMeshActor>(scene, "SkyboxActor");
            scene->AddActor(skyboxActor);

            StaticMeshComponent* skyboxMeshComponent = skyboxActor->GetMeshComponent();
            skyboxMeshComponent->SetStaticMesh(sphereMesh);

            skyboxMeshComponent->SetLocalPosition(Vec3(0, 0, 0));
            skyboxMeshComponent->SetLocalScale(Vec3(1, 1, 1) * 1000);

            {
                CE::Material* skyboxMaterial = CreateObject<CE::Material>(skyboxMeshComponent, "Material");
                skyboxMaterial->SetShader(skyboxShader.Get());
                skyboxMeshComponent->SetMaterial(skyboxMaterial, 0, 0);

                skyboxMaterial->SetProperty("_CubeMap", skybox.Get());
                skyboxMaterial->ApplyProperties();
            }

            Actor* sampleActor = CreateObject<Actor>(scene, "SampleActor");
            {
                SceneComponent* root = CreateObject<SceneComponent>(sampleActor, "Root");
                sampleActor->SetRootComponent(root);

                for (int i = 0; i < 4; ++i)
                {
                    SceneComponent* child = CreateObject<SceneComponent>(root, String::Format("Child_{}", i));
                    root->SetupAttachment(child);
                }

                scene->AddActor(sampleActor);
            }
        }

        gEditor->AddRenderViewport(viewport);
        gEngine->LoadScene(scene);

        Title(scene->GetName().GetString());
    }

    void SceneEditor::Construct()
    {
        Super::Construct();

        Title("Scene Editor");

        gEngine->GetSceneSubsystem()->AddCallbacks(this);

        ConstructMenuBar();
        ConstructDockspaces();

        LoadSandboxScene();

    }

    void SceneEditor::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        if (SceneSubsystem* sceneSubsystem = gEngine->GetSceneSubsystem())
        {
            sceneSubsystem->RemoveCallbacks(this);
        }
    }

    void SceneEditor::OnSceneLoaded(CE::Scene* scene)
    {
        sceneOutlinerTab->SetScene(scene);
    }

    void SceneEditor::OnAssetUnloaded(Uuid bundleUuid)
    {

    }


    void SceneEditor::OnSelectionChanged(FItemSelectionModel* selectionModel)
    {
        const auto& selection = selectionModel->GetSelection();

        Array<Actor*> selectedActors{};

        for (const FModelIndex& modelIndex : selection)
        {
            if (!modelIndex.IsValid())
                continue;

            Actor* actor = (Actor*)modelIndex.GetData().GetValue<Actor*>();
            selectedActors.Add(actor);
        }

        if (selectedActors.NotEmpty())
        {
            // TODO: Create and set object editor
            detailsTab->SetSelectedActor(selectedActors.GetLast());
        }
        else
        {
            detailsTab->SetSelectedActor(nullptr);
        }
    }

    void SceneEditor::ConstructDockspaces()
    {
        (*content)
        .Child(
            FAssignNew(FSplitBox, rootSplitBox)
            .Direction(FSplitDirection::Horizontal)
            .HAlign(HAlign::Fill)
            .VAlign(VAlign::Fill)
            (
                FNew(FSplitBox)
                .Direction(FSplitDirection::Vertical)
                .VAlign(VAlign::Fill)
                .FillRatio(0.75f)
                (
                    FAssignNew(EditorMinorDockspace, center)
                    .DockTabs(
                        FAssignNew(EditorViewportTab, viewportTab)

                    )
                    .HAlign(HAlign::Fill)
                    .FillRatio(0.6f),

                    FAssignNew(EditorMinorDockspace, bottom)
                    .DockTabs(
                        FAssignNew(AssetBrowser, assetBrowser),

                        FNew(EditorMinorDockTab)
                        .Title("Logs")
                        .Content(
                            FNew(FVerticalStack)
                            (
                                FNew(FTextButton)
                                .Text("Print Fusion Memory FootPrint")
                                .OnClicked([]
                                {
                                    u64 footprint = FusionApplication::Get()->ComputeMemoryFootprint();
                                    CE_LOG(Info, All, "Memory Footprint: {} KB", footprint / 1024);
                                })
                            )
                        )

                    )
                    .HAlign(HAlign::Fill)
                    .FillRatio(0.4f)
                ),

                FNew(FSplitBox)
                .Direction(FSplitDirection::Vertical)
                .VAlign(VAlign::Fill)
                .FillRatio(0.25f)
                (
                    FAssignNew(EditorMinorDockspace, rightTop)
                    .DockTabs(
                        FAssignNew(SceneOutlinerTab, sceneOutlinerTab)

                    )
                    .HAlign(HAlign::Fill)
                    .FillRatio(0.3f),

                    FAssignNew(EditorMinorDockspace, rightBottom)
                    .DockTabs(
                        FAssignNew(DetailsTab, detailsTab),

                        FNew(EditorMinorDockTab)
                        .Title("History")

                    )
                    .HAlign(HAlign::Fill)
                    .FillRatio(0.7f)
                )
            )
        )
    	.Padding(Vec4(0, 5, 0, 0));

        detailsTab->SetOwnerEditor(this);
        sceneOutlinerTab->SetOwnerEditor(this);
        viewportTab->SetOwnerEditor(this);

        viewportTab->GetViewport()->SetName("SceneEditorViewport");

        sceneOutlinerTab->treeView->SelectionModel()->OnSelectionChanged(FUNCTION_BINDING(this, OnSelectionChanged));
    }

    void SceneEditor::ConstructMenuBar()
    {
        (*menuBar)
            .Content(
                FNew(FMenuItem)
                .Text("File")
                .SubMenu(
                    FNew(EditorMenuPopup)
                    .Name("FileMenu")
                    .As<EditorMenuPopup>()
                    .Gap(0)
                    .Content(
                        FNew(FMenuItem)
                        .Text("New"),

                        FNew(FMenuItem)
                        .Text("Open"),

                        FNew(FMenuItem)
                        .Text("Open As..."),

                        FNew(FMenuItem)
                        .Text("Open Recent...")
                        .SubMenu(
                            FNew(EditorMenuPopup)
                            .Gap(0)
                            .Content(
                                FNew(FMenuItem)
                                .Text("Project 1"),

                                FNew(FMenuItem)
                                .Text("Project 2"),

                                FNew(FMenuItem)
                                .Text("Project 3"),

                                FNew(FMenuItem)
                                .Text("Project 4")
                            )
                            .Name("RecentsMenu")
                            .As<EditorMenuPopup>()
                        ),

                        FNew(FMenuItem)
                        .Text("Save"),

                        FNew(FMenuItem)
                        .Text("Save As..."),

                        FNew(FMenuItem)
                        .Text("Exit")
                        .OnClick([this]
                            {
                                GetContext()->QueueDestroy();
                            })
                    )
                )
                .Name("FileMenuItem"),

                FNew(FMenuItem)
                .Text("Edit")
                .SubMenu(
                    FNew(EditorMenuPopup)
                    .Name("EditMenu")
                    .As<EditorMenuPopup>()
                    .Gap(0)
                    .Content(
                        FNew(FMenuItem)
                        .Text("Project Settings")
                        .OnClick([this]
                        {
                            ProjectSettingsEditor::Show();
                        }),

                        FNew(FMenuItem)
                        .Text("Editor Settings")
                    )
                )
                .Name("EditMenuItem"),

                FNew(FMenuItem)
                .Text("Tools")
                .SubMenu(
                    FNew(EditorMenuPopup)
                    .Name("ToolsMenu")
                    .As<EditorMenuPopup>()
                    .Gap(0)
                    .Content(
                        FNew(FMenuItem)
                        .Text("Color Picker")
                        .OnClick([]
                        {
                            ColorPickerTool::Open()->OnColorChanged([](ColorPickerTool* colorPickerTool)
                            {
                                //CE_LOG(Info, All, "Color: {}", color.ToVec4());
                            });
                        }),

                        FNew(FMenuItem)
                        .Text("Debug")
                        .SubMenu(
                            FNew(EditorMenuPopup)
                            .Name("ToolsDebugMenu")
                            .As<EditorMenuPopup>()
                            .Content(
                                FNew(FMenuItem)
                                .Text("Fusion Image Atlas")
                                .OnClick([this]
                                {
                                    FusionImageAtlasWindow::Show();
                                }),

                                FNew(FMenuItem)
                                .Text("Fusion Font Atlas")
                                .OnClick([this]
                                {
                                    FusionFontAtlasWindow::Show();
                                })
                            )
                        )
                    )
                ),

                FNewNamed(FMenuItem, "HelpMenuItem")
                .Text("Help")
                .SubMenu(
					FNewNamed(EditorMenuPopup, "HelpMenu")
                    .As<EditorMenuPopup>()
                    .Gap(0)
                    .Content(
                        FNew(FMenuItem)
                        .Text("Fusion Samples")
                        .OnClick([this]
                        {
                            SampleWidgetWindow::Show();
                        }),

						FNew(FMenuItem)
                        .Text("About")
                        .OnClick([this]
                        {
                            AboutWindow::Show();
                        })
                    )
                )
            )
            ;
    }

}

