#include "EditorCore.h"

namespace CE::Editor
{

    MaterialAssetThumbnailGen::MaterialAssetThumbnailGen()
    {

    }

    MaterialAssetThumbnailGen::~MaterialAssetThumbnailGen()
    {
        
    }

    bool MaterialAssetThumbnailGen::StartProcessing()
    {
        if (IsProcessing())
            return false;

        AssetManager* assetManager = gEngine->GetAssetManager();

        Ref<TextureCube> skybox = assetManager->LoadAssetAtPath<TextureCube>("/Engine/Assets/Textures/HDRI/sample_day");

        Ref<CE::Shader> standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
        Ref<CE::Shader> skyboxShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/SkyboxCubeMap");

        for (const auto& assetPath : assetPaths)
        {
            Ref<CE::MaterialInterface> material = assetManager->LoadAssetAtPath<CE::MaterialInterface>(assetPath);
            if (!material)
				continue;

            Ref<CE::Scene> viewportScene = CreateObject<CE::Scene>(this, FixObjectName("OffscreenScene_" + material->GetName().GetString()));
            viewportScene->SetSkyboxCubeMap(skybox.Get());

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
                auto sphereMeshComponent = sphereActor->GetMeshComponent();
                sphereMeshComponent->SetStaticMesh(sphereMesh);
                sphereMeshComponent->SetLocalPosition(Vec3(0, 0, 1.f));
                sphereMeshComponent->SetLocalEulerAngles(Vec3(0, 0, 0));
                sphereMeshComponent->SetMaterial(material.Get(), 0, 0);
            }

            Ref<CameraActor> camera = CreateObject<CameraActor>(viewportScene.Get(), "Mat_Camera");
            camera->GetCameraComponent()->SetLocalPosition(Vec3(0, 0, 0));
            camera->GetCameraComponent()->GetRenderPipeline()->clearColor = Color::RGBA(0, 0, 0, 0);
            camera->GetCameraComponent()->GetRenderPipeline()->MarkDirty();
            viewportScene->AddActor(camera.Get());

            Ref<SceneRenderer> sceneRenderer = CreateObject<SceneRenderer>(this, "SceneRenderer");
            sceneRenderer->SetScene(viewportScene);
            sceneRenderer->SetOneShot(true);
            sceneRenderers.Add(sceneRenderer);

            u32 thumbnailResolution = this->thumbnailResolution;

            sceneRenderer->onRenderFinished += [thumbnailResolution, assetPath, this](Ref<SceneRenderer> sceneRenderer)
                {
                    CE_LOG(Info, All, "Offscreen scene rendered!");

                    RHI::BufferDescriptor bufferDescriptor{};
                    bufferDescriptor.name = "Staging Buffer";
                    bufferDescriptor.defaultHeapType = MemoryHeapType::ReadBack;
                    bufferDescriptor.bindFlags = BufferBindFlags::StagingBuffer;
                    bufferDescriptor.bufferSize = thumbnailResolution * thumbnailResolution * 4;

                    RHI::Buffer* stagingBuffer = RHI::gDynamicRHI->CreateBuffer(bufferDescriptor);

                    RHI::Texture* image = sceneRenderer->GetOutputImage(0);

                    RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
                    RHI::CommandList* cmdList = RHI::gDynamicRHI->AllocateCommandList(queue, CommandListType::Direct);
                    RHI::Fence* fence = RHI::gDynamicRHI->CreateFence();

                    cmdList->Begin();
                    {
                        RHI::ResourceBarrierDescriptor barrier{};
                        barrier.subresourceRange = RHI::SubresourceRange::All();
                        barrier.resource = image;
                        barrier.fromState = ResourceState::ColorOutput;
                        barrier.toState = ResourceState::CopySource;
                        cmdList->ResourceBarrier(1, &barrier);

                        barrier.resource = stagingBuffer;
                        barrier.fromState = ResourceState::Undefined;
                        barrier.toState = ResourceState::CopyDestination;
                        cmdList->ResourceBarrier(1, &barrier);

                        RHI::TextureToBufferCopy copyRegion;
                        copyRegion.srcTexture = image;
                        copyRegion.baseArrayLayer = 0;
                        copyRegion.layerCount = 1;
                        copyRegion.mipSlice = 0;
                        copyRegion.dstBuffer = stagingBuffer;
                        copyRegion.bufferOffset = 0;

                        cmdList->CopyTextureRegion(copyRegion);

                        barrier.resource = image;
                        barrier.fromState = ResourceState::CopySource;
                        barrier.toState = ResourceState::ColorOutput;
                        cmdList->ResourceBarrier(1, &barrier);

                    }
                    cmdList->End();

                    queue->Execute(1, &cmdList, fence);
                    fence->WaitForFence();

                    RHI::gDynamicRHI->DestroyFence(fence);
                    RHI::gDynamicRHI->FreeCommandLists(1, &cmdList);

                    void* data;
                    stagingBuffer->Map(0, stagingBuffer->GetBufferSize(), &data);
                    {
                        SaveThumbnailToDisk(data, thumbnailResolution, thumbnailResolution, assetPath);
                    }
                    stagingBuffer->Unmap();

                    RHI::gDynamicRHI->DestroyBuffer(stagingBuffer);
                    RHI::gDynamicRHI->DestroyTexture(image);

                    OnJobFinished(sceneRenderer);
                };

            RHI::TextureDescriptor desc{};
            desc.width = desc.height = thumbnailResolution;
            desc.depth = 1;
            desc.format = RHI::Format::R8G8B8A8_UNORM;
            desc.name = "Offscreen Scene";
            desc.dimension = Dimension::Dim2D;
            desc.bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead;
            desc.sampleCount = 1;

            RHI::Texture* image = RHI::gDynamicRHI->CreateTexture(desc);

            for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; ++i)
            {
                sceneRenderer->SetOutputImage(i, image);
            }

            totalJobs++;
            gEngine->EnqueueSceneRenderer(sceneRenderer);
        }

        return true;
    }

    bool MaterialAssetThumbnailGen::IsProcessing()
    {
        return totalJobs > 0;
    }

    bool MaterialAssetThumbnailGen::IsValidForAssetType(SubClass<Asset> assetClass)
    {
        return assetClass->IsSubclassOf<CE::MaterialInterface>();
    }

    void MaterialAssetThumbnailGen::OnJobFinished(Ref<SceneRenderer> sceneRenderer)
    {
        totalJobs--;

        sceneRenderers.Remove(sceneRenderer);
        {
	        if (Ref<CE::Scene> scene = sceneRenderer->GetScene())
            {
                scene->BeginDestroy();
                sceneRenderer->SetScene(nullptr);
            }
        }
        sceneRenderer->BeginDestroy();

        if (totalJobs <= 0)
        {
            totalJobs = 0;
            OnFinish();
		}
    }


} // namespace CE

