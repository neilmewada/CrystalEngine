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

    bool SceneEditor::OpenEditor(Ref<Object> targetObject, Ref<Bundle> bundle)
    {
        ZoneScoped;

    	if (!targetObject || !bundle)
            return false;
        if (!CanEdit(targetObject))
            return false;

        Ref<CE::Scene> scene = CastTo<CE::Scene>(targetObject);
        if (!scene)
            return false;

        if (!OpenScene(scene))
        {
	        return false;
        }

    	Super::OpenEditor(targetObject, bundle);
        return true;
    }

    bool SceneEditor::OpenScene(Ref<CE::Scene> scene)
    {
        ZoneScoped;

        if (!scene)
            return false;
        if (!scene->GetRpiScene())
            return false;

        if (targetScene.IsValid() && targetScene->GetOuter() == this)
        {
            targetScene->BeginDestroy();
        }

        targetScene = scene;

        EditorViewport* viewport = viewportTab->GetViewport();
        viewport->SetScene(scene->GetRpiScene());

        sceneOutlinerTab->SetScene(scene.Get());
        detailsTab->SetSelectedActor(nullptr);
        
        return true;
    }

    SceneEditor::SceneEditor()
    {
        m_CanBeUndocked = false;

        addActorContextMenu = CreateDefaultSubobject<EditorMenuPopup>("AddActorMenu");
    }

    void SceneEditor::LoadSandboxScene()
    {
        ZoneScoped;

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

			CE::Material* arrowMaterial = CreateObject<CE::Material>(scene, "ArrowMaterial");
            arrowMaterial->SetShader(standardShader.Get());
            {
                arrowMaterial->SetProperty("_Albedo", Colors::White);
                arrowMaterial->ApplyProperties();
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
                meshComponent->SetLocalScale(Vec3(50, 0.1f, 50));
                meshComponent->SetMaterial(woodMaterial, 0, 0);
            }

            if (true)
            {
                carActor->SetEnabled(false);
                chairActor->SetEnabled(false);
            }

            if (false)
	        {
		        if (Ref<StaticMesh> arrowMesh = assetManager->LoadAssetAtPath<StaticMesh>("/Engine/Assets/Models/SM_Editor_Arrow"))
		        {
		        	StaticMeshActor* arrowActor = CreateObject<StaticMeshActor>(scene, "ArrowMesh");
		        	scene->AddActor(arrowActor);
			        {
		        		StaticMeshComponent* meshComponent = arrowActor->GetMeshComponent();
		        		meshComponent->SetStaticMesh(arrowMesh);
		        		meshComponent->SetLocalPosition(Vec3(0, 0, 5));
		        		meshComponent->SetLocalEulerAngles(Vec3(0, 0, 0));
		        		meshComponent->SetLocalScale(Vec3(1, 1, 1));
		        		meshComponent->SetMaterial(arrowMaterial, 0, 0);
			        }
		        }
	        }

            CameraActor* camera = CreateObject<CameraActor>(scene, "Camera");
            scene->AddActor(camera);

            CameraComponent* cameraComponent = camera->GetCameraComponent();
            cameraComponent->SetLocalPosition(Vec3(0, 5, -5));
            cameraComponent->SetLocalEulerAngles(Vec3(35, 0, 0));
            cameraComponent->SetFieldOfView(60);

            DirectionalLight* lightActor = CreateObject<DirectionalLight>(scene, "Sun");
            scene->AddActor(lightActor);
            {
                DirectionalLightComponent* directionalLight = lightActor->GetDirectionalLightComponent();
                directionalLight->SetLocalEulerAngles(Vec3(30, 0, 0));
                directionalLight->SetIntensity(10);
                directionalLight->SetLightColor(Colors::White);
            }

            lightActor->SetEnabled(false);

            constexpr int NumLights = 9;
            constexpr std::array<Vec3, NumLights> LightPos = { Vec3(-2, 0, -2), Vec3(0, 0, -2), Vec3(2, 0, -2),
	            Vec3(-2, 0, 0), Vec3(0, 0, 0), Vec3(2, 0, 0),
	            Vec3(-2, 0, 2), Vec3(0, 0, 2), Vec3(2, 0, 2) };
            constexpr std::array<Color, NumLights> LightColors = { Colors::Red, Colors::Green, Colors::Blue,
                Colors::Yellow, Colors::Cyan, Colors::Magenta,
				Colors::White, Colors::Orange, Colors::Purple };
			constexpr float Separation = 3.0f;
			constexpr Vec2i LightGrid = Vec2i(16, 16);
			int lightIndex = 0;
            
            for (int x = -LightGrid.x / 2; x <= LightGrid.x / 2; ++x)
            {
                for (int y = -LightGrid.y / 2; y <= LightGrid.y / 2; ++y)
                {
                    PointLight* pointLight = CreateObject<PointLight>(scene, "PointLight");
                    scene->AddActor(pointLight);
                    {
                        Ref<PointLightComponent> pointLightComponent = pointLight->GetPointLightComponent();
                        pointLightComponent->SetLocalPosition(Vec3(x * Separation, -0.5f, y * Separation + 5));
                        pointLightComponent->SetRange(2.0f);
                        pointLightComponent->SetLightColor(LightColors[lightIndex]);
                        pointLightComponent->SetIntensity(25);
                    }

					lightIndex = (lightIndex + 1) % NumLights;
                }
			}

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

    void SceneEditor::LoadEmptyScene()
    {
        Ref<CE::Scene> scene = CreateObject<CE::Scene>(this, "UntitledScene");

        OpenScene(scene);
    }

    void SceneEditor::Construct()
    {
        ZoneScoped;

        Super::Construct();

        Title("Scene Editor");

        gEngine->GetSceneSubsystem()->AddCallbacks(this);

        ConstructDockspaces();
    	LoadSandboxScene();
        //LoadEmptyScene();
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

    void SceneEditor::OnActorSelectionChanged(FItemSelectionModel* selectionModel)
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
            detailsTab->SetSelectedActor(selectedActors.GetLast());
        }
        else
        {
            detailsTab->SetSelectedActor(nullptr);
        }
    }

    void SceneEditor::OnClickPlay()
    {
        return;

        gEngine->GetSceneSubsystem()->PlayActiveScene();
        sandboxScene->GetPhysicsScene()->SetSimulationEnabled(true);

        playButton->Enabled(false);
        pauseButton->Enabled(true);
        stopButton->Enabled(true);
    }

    void SceneEditor::OnClickPause()
    {
        return;

        sandboxScene->GetPhysicsScene()->SetSimulationEnabled(false);

        playButton->Enabled(true);
        pauseButton->Enabled(false);
        stopButton->Enabled(true);
    }

    void SceneEditor::OnClickStop()
    {
        return;

        // TODO: Restore scene to original state

        playButton->Enabled(true);
        pauseButton->Enabled(false);
        stopButton->Enabled(false);
    }

    void SceneEditor::OnClickAddActorMenuButton()
    {
        if (!GetContext())
            return;

        //addActorContextMenu->ClosePopup();
        addActorContextMenu->DestroyAllItems();

        (*addActorContextMenu)
            .Content(
                FNew(FMenuItemSeparator)
                .Title("BASIC"),

                EditorMenuPopup::NewMenuItem()
                .Text("Actor")
                .OnClick([this]
                {
	                if (targetScene)
	                {
                        Ref<Actor> actor = CreateObject<Actor>(targetScene.Get(), "EmptyActor");
                        targetScene->AddActor(actor.Get());
	                }
                }),

                FNew(FMenuItemSeparator)
                .Title("SHAPES"),

                EditorMenuPopup::NewMenuItem()
                .Text("Sphere")
            );

        Vec2 popupPos = addActorButton->GetGlobalPosition() + Vec2(0, addActorButton->GetComputedSize().y);

        GetContext()->PushLocalPopup(addActorContextMenu.Get(), popupPos, Vec2(), addActorButton->GetComputedSize());
    }

    void SceneEditor::ConstructDockspaces()
    {
        ZoneScoped;

        minorDockspace->AddDockWindow(
            FAssignNew(EditorViewportTab, viewportTab)
        );

        auto rootSplit = minorDockspace->GetRootSplit();

        viewportTab->GetDockspaceSplitView()->AddDockWindowSplit(FDockingHintPosition::Right,
            FAssignNew(SceneOutlinerTab, sceneOutlinerTab),
            0.35f
        );

        sceneOutlinerTab->GetDockspaceSplitView()->AddDockWindowSplit(FDockingHintPosition::Bottom,
			FAssignNew(ActorDetailsTab, detailsTab),
            0.65f
        );

        viewportTab->GetDockspaceSplitView()->AddDockWindowSplit(FDockingHintPosition::Bottom,
			FAssignNew(AssetBrowser, assetBrowser),
            0.4f
        );

        detailsTab->SetOwnerEditor(this);
        sceneOutlinerTab->SetOwnerEditor(this);
        viewportTab->SetOwnerEditor(this);

        viewportTab->GetViewport()->SetName("SceneEditorViewport");

        sceneOutlinerTab->treeView->SelectionModel()->OnSelectionChanged(FUNCTION_BINDING(this, OnActorSelectionChanged));
    }

    void SceneEditor::ConstructMenuBar()
    {
        ZoneScoped;

        Super::ConstructMenuBar();

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
                                }),

                                FNew(FMenuItem)
                                .Text("Fusion SDF Font Atlas")
                                .OnClick([this]
                                {
                                    FusionSDFFontAtlasWindow::Show();
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

    void SceneEditor::ConstructToolBar()
    {
        ZoneScoped;

        Super::ConstructToolBar();

        (*toolBar)
        .Content(
            EditorToolBar::NewImageButton("/Editor/Assets/Icons/Save")
            .OnClicked(FUNCTION_BINDING(this, SaveChanges)),

            EditorToolBar::NewSeparator(),

            EditorToolBar::NewImageButton("/Editor/Assets/Icons/AddObject")
            .Assign(addActorButton)
            .OnClicked(FUNCTION_BINDING(this, OnClickAddActorMenuButton))/*,

            EditorToolBar::NewImageButton("/Editor/Assets/Icons/Play")
            .Assign(playButton)
            .OnClicked(FUNCTION_BINDING(this, OnClickPlay)),

            EditorToolBar::NewImageButton("/Editor/Assets/Icons/Pause")
            .Assign(pauseButton)
            .OnClicked(FUNCTION_BINDING(this, OnClickPause))
            .Enabled(false),

            EditorToolBar::NewImageButton("/Editor/Assets/Icons/Stop")
            .Assign(stopButton)
            .OnClicked(FUNCTION_BINDING(this, OnClickStop))
            .Enabled(false)*/
		);
    }
}

