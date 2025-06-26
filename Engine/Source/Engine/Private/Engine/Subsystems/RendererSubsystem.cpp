#include "Engine.h"

namespace CE
{

    RendererSubsystem::RendererSubsystem()
    {
		
    }

    void RendererSubsystem::RebuildFrameGraph()
    {
		rebuildFrameGraph = recompileFrameGraph = true;

    }

    f32 RendererSubsystem::GetTickPriority() const
    {
		return 2;
    }

	void RendererSubsystem::OnWindowShown(PlatformWindow* window)
	{
		RebuildFrameGraph();
	}

    void RendererSubsystem::OnWindowCreated(PlatformWindow* window)
    {
		RebuildFrameGraph();
    }

    void RendererSubsystem::OnWindowDestroyed(PlatformWindow* window)
    {
		RebuildFrameGraph();
    }

    void RendererSubsystem::OnWindowClosed(PlatformWindow* window)
    {
		RebuildFrameGraph();
    }

    void RendererSubsystem::OnWindowMinimized(PlatformWindow* window)
    {
		RebuildFrameGraph();
    }

    void RendererSubsystem::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
    {
		RebuildFrameGraph();
    }

    void RendererSubsystem::OnWindowRestored(PlatformWindow* window)
    {
		RebuildFrameGraph();
    }

	void RendererSubsystem::OnWindowExposed(PlatformWindow* window)
	{
		auto id = window->GetWindowId();
		Vec2i windowSize = window->GetWindowSize();

		if (!windowSizesById.KeyExists(id) || windowSize != windowSizesById[id])
		{
			RebuildFrameGraph();

			scheduler->ResetFramesInFlight();
		}

		windowSizesById[id] = windowSize;
	}

    void RendererSubsystem::Initialize()
	{
		Super::Initialize();

		auto app = FusionApplication::TryGet();

		if (app)
		{
			app->onRenderViewportDestroyed.Bind(FUNCTION_BINDING(this, RemoveViewport));
		}
	}

	void RendererSubsystem::PostInitialize()
	{
		Super::PostInitialize();

		sceneSubsystem = gEngine->GetSubsystem<SceneSubsystem>();

		// - Feature Processors -

		RegisterFeatureProcessor<StaticMeshComponent, RPI::StaticMeshFeatureProcessor>();

		// - Scheduler -

		RHI::FrameSchedulerDescriptor desc{};
		desc.numFramesInFlight = 2;

		scheduler = RHI::FrameScheduler::Create(desc);

		PlatformWindow* mainWindow = PlatformApplication::Get()->GetMainWindow();

		// TODO: Implement multi scene support
		CE::Scene* mainScene = sceneSubsystem->GetActiveScene();

		if (mainWindow)
		{
			PlatformApplication::Get()->AddMessageHandler(this);

			FusionInitInfo initInfo{};
			initInfo.assetLoader = gEngine->GetAssetManager();

			FusionApplication::Get()->Initialize(initInfo);

			//activeScene->mainRenderViewport = primaryViewport;
		}
	}

	void RendererSubsystem::PreShutdown()
	{
		ZoneScoped;

		Super::PreShutdown();

		FusionApplication* app = FusionApplication::TryGet();

		if (app)
		{
			PlatformApplication::Get()->RemoveMessageHandler(this);

			app->Shutdown();
			app->BeginDestroy();
		}

		delete scheduler; scheduler = nullptr;
	}

	void RendererSubsystem::Shutdown()
	{
		Super::Shutdown();

	}

	void RendererSubsystem::Tick(f32 delta)
	{
		ZoneScoped;

		Super::Tick(delta);

		FusionApplication* app = FusionApplication::TryGet();

		int submittedImageIndex = -1;

		if (app)
		{
			app->Tick();
		}

		if (IsEngineRequestingExit())
		{
			return;
		}

		if (rebuildFrameGraph)
		{
			rebuildFrameGraph = false;
			recompileFrameGraph = true;

			BuildFrameGraph();
			submittedImageIndex = curImageIndex;
		}

		if (recompileFrameGraph)
		{
			recompileFrameGraph = false;

			CompileFrameGraph();
		}

		if (IsEngineRequestingExit())
		{
			return;
		}

		//CE::Scene* scene = sceneSubsystem->GetActiveScene();
		//RPI::Scene* rpiScene = scene->GetRpiScene();
		constexpr bool isSceneWindowActive = false;

		if (rebuildFrameGraph || recompileFrameGraph)
		{
			RebuildFrameGraph();
			return;
		}

		int imageIndex = scheduler->BeginExecution();

		if (imageIndex >= RHI::Limits::MaxSwapChainImageCount || rebuildFrameGraph || recompileFrameGraph)
		{
			RebuildFrameGraph();
			return;
		}

    	for (FGameWindow* renderViewport : renderViewports)
    	{
    		RPI::Scene* rpiScene = renderViewport->GetScene();
    		CE::Scene* scene = sceneSubsystem->FindRpiSceneOwner(rpiScene);
    		if (scene == nullptr || !scene->IsEnabled())// || !renderViewport->IsEnabledInHierarchy())
    		{
    			// A viewport that was previously visible is no longer visible!
    			if (previouslyVisibleViewports.Exists(renderViewport->GetUuid()))
    			{
    				previouslyVisibleViewports.Remove(renderViewport->GetUuid());

    				RebuildFrameGraph();
    				return;
    			}
    			continue;
    		}

    		// A viewport that was previously disabled is now enabled!
    		if (!previouslyVisibleViewports.Exists(renderViewport->GetUuid()))
    		{
    			previouslyVisibleViewports.Add(renderViewport->GetUuid());

    			RebuildFrameGraph();
    			return;
    		}
    	}

		curImageIndex = imageIndex;

		// ---------------------------------------------------------
		// - Enqueue draw packets to views

		if (submittedImageIndex != curImageIndex)
		{
			RPI::RPISystem::Get().SimulationTick(curImageIndex);
			RPI::RPISystem::Get().RenderTick(curImageIndex);
		}

		// ---------------------------------------------------------
		// - Submit draw lists to scopes for execution

		drawList.Shutdown();

		RHI::DrawListMask drawListMask{};
		HashSet<RHI::DrawListTag> drawListTags{};

		// - Setup draw list mask

		if (app)
		{
			app->UpdateDrawListMask(drawListMask);
		}

		for (FGameWindow* renderViewport : renderViewports)
		{
			//if (!renderViewport->IsEnabledInHierarchy())
			//	continue;

			renderViewport->GetDrawListContext().Shutdown();

			RPI::Scene* rpiScene = renderViewport->GetScene();
			if (!rpiScene)
				continue;

			for (int i = 0; i < rpiScene->GetRenderPipelineCount(); ++i)
			{
				RPI::RenderPipeline* renderPipeline = rpiScene->GetRenderPipeline(i);
				if (!renderPipeline)
					continue;

				renderPipeline->GetPassTree()->IterateRecursively([&](RPI::Pass* pass)
					{
						if (!pass)
							return;

						if (pass->GetDrawListTag().IsValid())
						{
							renderViewport->GetDrawListMask().Set(pass->GetDrawListTag());
							drawListMask.Set(pass->GetDrawListTag());
						}
					});
			}
		}

		// - Enqueue additional draw packets

		for (int i = 0; i < drawListMask.GetSize(); ++i)
		{
			if (drawListMask.Test(i))
			{
				drawListTags.Add((u8)i);
			}
		}

		drawList.Init(drawListMask);

		if (app)
		{
			app->EnqueueDrawPackets(drawList, curImageIndex);
		}

		for (FGameWindow* renderViewport : renderViewports)
		{
			//if (!renderViewport->IsEnabledInHierarchy())
			//	continue;

			RPI::Scene* rpiScene = renderViewport->GetScene();
			if (!rpiScene)
				continue;

			renderViewport->GetDrawListContext().Init(renderViewport->GetDrawListMask());

			for (const auto& [viewTag, views] : rpiScene->GetViews())
			{
				for (RPI::View* view : views.views)
				{
					view->GetDrawListContext()->Finalize();

					for (const auto& drawListTag : drawListTags)
					{
						RHI::DrawList& viewDrawList = view->GetDrawList(drawListTag);
						for (int i = 0; i < viewDrawList.GetDrawItemCount(); ++i)
						{
							renderViewport->GetDrawListContext().AddDrawItem(viewDrawList.GetDrawItem(i), drawListTag);
							//drawList.AddDrawItem(viewDrawList.GetDrawItem(i), drawListTag);
						}
					}

					if (viewTag == "DirectionalLightShadow")
					{
						break; // Only 1 view allowed for Directional Light Shadows
					}
				}
			}

			renderViewport->GetDrawListContext().Finalize();
		}

		drawList.Finalize();

		// - Set scope draw lists
    
		if (app) // FWidget Scopes & DrawLists
		{
			app->FlushDrawPackets(drawList, curImageIndex);
		}

		for (FGameWindow* renderViewport : renderViewports)
		{
			//if (!renderViewport->IsEnabledInHierarchy())
			//	continue;

			RPI::Scene* rpiScene = renderViewport->GetScene();
			if (!rpiScene)
				continue;

			for (int i = 0; i < rpiScene->GetRenderPipelineCount(); ++i)
			{
				RPI::RenderPipeline* renderPipeline = rpiScene->GetRenderPipeline(i);
				if (!renderPipeline)
					continue;

				renderPipeline->GetPassTree()->IterateRecursively([&](RPI::Pass* pass)
					{
						if (!pass)
							return;

						RPI::SceneViewTag viewTag = pass->GetViewTag();
						RHI::DrawListTag passDrawTag = pass->GetDrawListTag();
						RHI::ScopeId scopeId = pass->GetScopeId();

						if (passDrawTag.IsValid() && viewTag.IsValid() && scopeId.IsValid())
						{
							// TODO: Fix this for multi-scene setups. Currently it grabs ALL of the draw items based on a tag.
							// Which means it'll grab DrawItems from multiple scenes and potentially override previous ones!
							DrawList& viewportDrawList = renderViewport->GetDrawListContext().GetDrawListForTag(passDrawTag);
							scheduler->SetScopeDrawList(scopeId, &viewportDrawList);
						}
					});
			}
		}

		scheduler->EndExecution();
	}

	void RendererSubsystem::BuildFrameGraph()
	{
		ZoneScoped;

		rebuildFrameGraph = false;
		recompileFrameGraph = true;

		// TODO: Implement multi scene support, and also multi-viewport support

		RPI::RPISystem::Get().SimulationTick(curImageIndex);
		RPI::RPISystem::Get().RenderTick(curImageIndex);

		RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();
    	
		scheduler->BeginFrameGraph();
		{
			auto app = FusionApplication::TryGet();

			if (app)
			{
				// Emplace attachments from Fusion
				app->EmplaceFrameAttachments();

				// Cleanup first
				previouslyVisibleViewports.Clear();

				for (FGameWindow* renderViewport : renderViewports)
				{
					Ref<FNativeContext> nativeContext = CastTo<FNativeContext>(renderViewport->GetContext());
					if (!nativeContext)
						continue;
					//if (!renderViewport->IsEnabledInHierarchy())
					//	continue;

					nativeContext->shaderReadOnlyAttachmentDependencies.Clear();
					nativeContext->shaderWriteAttachmentDependencies.Clear();
				}

				for (FGameWindow* renderViewport : renderViewports)
				{
					RPI::Scene* rpiScene = renderViewport->GetScene();
					CE::Scene* scene = sceneSubsystem->FindRpiSceneOwner(rpiScene);
					if (scene == nullptr || !scene->IsEnabled())
						continue;
					//if (!renderViewport->IsEnabledInHierarchy())
					//	continue;

					previouslyVisibleViewports.Add(renderViewport->GetUuid());

					Ref<FNativeContext> nativeContext = renderViewport->GetNativeContext();
					if (!nativeContext)
					{
						continue;
					}

					PlatformWindow* platformWindow = nativeContext->GetPlatformWindow();
					if (platformWindow && (platformWindow->IsHidden() || platformWindow->IsMinimized()))
					{
						continue;
					}

					for (CE::RenderPipeline* renderPipeline : scene->renderPipelines)
					{
						RPI::RenderPipeline* rpiPipeline = renderPipeline->GetRpiRenderPipeline();
						const auto& attachments = rpiPipeline->attachments;

						for (RPI::PassAttachment* passAttachment : attachments)
						{
							if (passAttachment->lifetime == RHI::AttachmentLifetimeType::External && passAttachment->name == "PipelineOutput")
							{
								if (renderViewport->IsEmbeddedViewport())
								{
									FViewport* viewport = static_cast<FViewport*>(renderViewport);

									passAttachment->attachmentId = String::Format("Viewport_{}", viewport->GetUuid());

									StaticArray<RHI::TextureView*, RHI::Limits::MaxSwapChainImageCount> frameBuffer{};

									RHI::ImageScopeAttachmentDescriptor descriptor{};
									descriptor.attachmentId = passAttachment->attachmentId;
									descriptor.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
									descriptor.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;

									nativeContext->shaderReadOnlyAttachmentDependencies.Add(descriptor);

									for (int i = 0; i < frameBuffer.GetSize(); ++i)
									{
										if (viewport->GetFrame(i) == nullptr)
										{
											viewport->RecreateFrameBuffer();
										}

										//frameBuffer[i] = viewport->GetFrame(i)->GetRhiTexture();
										frameBuffer[i] = viewport->GetFrameView(i);
									}

									attachmentDatabase.EmplaceFrameAttachment(passAttachment->attachmentId, frameBuffer);
								}
								else
								{
									passAttachment->attachmentId = nativeContext->attachmentId;
								}
							}
							else
							{
								passAttachment->attachmentId = String::Format("{}_{}", passAttachment->name, rpiPipeline->uuid);
							}
						}

						// Emplace all attachments and then add scopes from the pipeline
						rpiPipeline->ImportScopeProducers(scheduler);
					}
				}

				// Enqueue Scopes from Fusion
				app->EnqueueScopes();
			}
		}
		scheduler->EndFrameGraph();
	}

	void RendererSubsystem::CompileFrameGraph()
	{
		ZoneScoped;

		recompileFrameGraph = false;

		scheduler->Compile();

		RHI::TransientMemoryPool* pool = scheduler->GetTransientPool();
		RHI::MemoryHeap* imageHeap = pool->GetImagePool();
		if (imageHeap != nullptr)
		{
			//CE_LOG(Info, All, "Transient Image Pool: {} MB", (imageHeap->GetHeapSize() / 1024.0f / 1024.0f));
		}
	}

	void RendererSubsystem::SubmitDrawPackets(int imageIndex)
	{
		
	}

	void RendererSubsystem::RegisterFeatureProcessor(SubClass<ActorComponent> componentClass,
		SubClass<RPI::FeatureProcessor> fpClass)
	{
		if (componentClass == nullptr || fpClass == nullptr)
			return;

		componentClassToFeatureProcessorClass[componentClass] = fpClass;
	}

	SubClass<RPI::FeatureProcessor> RendererSubsystem::GetFeatureProcessClass(SubClass<ActorComponent> componentClass)
	{
		if (componentClass == nullptr)
			return nullptr;

		Class* parentClass = componentClass;

		while (parentClass != nullptr)
		{
			if (parentClass->GetSuperClassCount() == 0 || parentClass == GetStaticClass<Object>())
				break;

			if (componentClassToFeatureProcessorClass.KeyExists(parentClass))
			{
				return componentClassToFeatureProcessorClass[parentClass];
			}
			parentClass = parentClass->GetSuperClass(0);
		}

		return nullptr;
	}

	void RendererSubsystem::AddViewport(FGameWindow* viewport)
	{
		if (renderViewports.Exists(viewport))
		{
			RebuildFrameGraph();
			return;
		}

		renderViewports.Add(viewport);
		RebuildFrameGraph();

		if (viewport->IsEmbeddedViewport())
		{
			FViewport* embeddedViewport = static_cast<FViewport*>(viewport);
			embeddedViewport->OnFrameBufferRecreated(FUNCTION_BINDING(this, RebuildFrameGraph));
		}
	}

	void RendererSubsystem::RemoveViewport(FGameWindow* viewport)
	{
		if (renderViewports.Remove(viewport))
		{
			RebuildFrameGraph();
		}
	}

	void RendererSubsystem::OnSceneDestroyed(CE::Scene* scene)
	{
		if (!scene)
			return;

		for (FGameWindow* renderViewport : renderViewports)
		{
			if (renderViewport != nullptr && renderViewport->GetScene() == scene->GetRpiScene())
			{
				renderViewports.Remove(renderViewport);
				return;
			}
		}
	}

} // namespace CE
