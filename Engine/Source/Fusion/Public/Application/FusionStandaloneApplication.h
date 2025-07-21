#pragma once

namespace CE
{

    class FUSION_API FusionStandaloneApplication : public ApplicationMessageHandler
    {
    public:
        FusionStandaloneApplication();

        virtual ~FusionStandaloneApplication();

		void Run(Ref<FWindow> mainWindow, u32 width, u32 height, const PlatformWindowInfo& windowInfo);

		template<class TWindow> requires TIsBaseClassOf<FWindow, TWindow>::Value
		void Launch(u32 width, u32 height, const PlatformWindowInfo& windowInfo)
		{
			auto app = PlatformApplication::Get();

			f32 scaleFactor = GetDefaults<FusionApplication>()->GetDefaultScalingFactor();

#if PLATFORM_MAC
			u32 w = width, h = height;
#elif PLATFORM_LINUX
			u32 w = width * scaleFactor, h = height * scaleFactor;
#elif PLATFORM_WINDOWS
			u32 w = width, h = height;
#endif

			PlatformWindow* window = app->InitMainWindow("MainWindow", w, h, windowInfo);
			window->SetBorderless(true);

			InputManager::Get().Initialize(app);

			CreateRHI();

			RHI::gDynamicRHI->Initialize();
			RHI::gDynamicRHI->PostInitialize();

			RHI::FrameSchedulerDescriptor schedulerDesc{};
			schedulerDesc.numFramesInFlight = 2;
			RHI::FrameScheduler::Create(schedulerDesc);

			RPI::RPISystem::Get().Initialize();

			FusionApplication* fApp = FusionApplication::Get();

			FusionInitInfo initInfo = {};
			initInfo.assetLoader = nullptr;
			initInfo.rebuildFrameGraphMethod = [this]
				{
					RebuildFrameGraph();
				};
			fApp->Initialize(initInfo);

			JobManagerDesc desc{};
			desc.defaultTag = JOB_THREAD_WORKER;
			desc.totalThreads = 0; // auto set optimal number of threads
			desc.threads.Resize(4, { .threadName = "FusionThread", .tag = JOB_THREAD_FUSION });

			jobManager = new JobManager("JobSystemManager", desc);
			jobContext = new JobContext(jobManager);
			JobContext::PushGlobalContext(jobContext);

			Logger::Initialize();

			PlatformApplication::Get()->AddMessageHandler(this);

			scheduler = RHI::FrameScheduler::Get();

			SetupDefaultStyle();

			FRootContext* rootContext = FusionApplication::Get()->GetRootContext();

			FNativeContext* nativeContext = FNativeContext::Create(window, "TestWindow", rootContext);
			rootContext->AddChildContext(nativeContext);

			TWindow* mainWidget;
			FAssignNewOwned(TWindow, mainWidget, nativeContext);

			nativeContext->SetOwningWidget(mainWidget);

			auto exposedTick = [&]
				{
					FusionApplication::Get()->SetExposed();

					FusionApplication::Get()->Tick();

					Render();
				};

			DelegateHandle handle = PlatformApplication::Get()->AddTickHandler(exposedTick);

			window->Show();

			int frameCounter = 0;

			while (!IsEngineRequestingExit())
			{
				auto curTime = clock();
				deltaTime = (f32)(curTime - previousTime) / CLOCKS_PER_SEC;

				FusionApplication::Get()->ResetExposed();

				// App & Input Tick
				PlatformApplication::Get()->Tick();
				InputManager::Get().Tick();

				Render();

				previousTime = curTime;
				frameCounter++;
			}

			PlatformApplication::Get()->RemoveTickHandler(handle);

			FStyleManager* styleManager = fApp->GetStyleManager();
			styleManager->DeregisterStyleSet(rootStyle->GetName());
		}

    private:

		void CreateRHI();

		void SetupDefaultStyle();

		void Init();
		void Shutdown();

		void Render();

		void RebuildFrameGraph();

		void BuildFrameGraph();
		void CompileFrameGraph();

		void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowDestroyed(PlatformWindow* window) override;
		void OnWindowClosed(PlatformWindow* window) override;
		void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;
		void OnWindowMinimized(PlatformWindow* window) override;
		void OnWindowCreated(PlatformWindow* window) override;
		void OnWindowExposed(PlatformWindow* window) override;

		HashMap<u64, Vec2i> windowSizesById;
		RHI::FrameScheduler* scheduler = nullptr;

		bool rebuildFrameGraph = true;
		bool recompileFrameGraph = true;
		int curImageIndex = 0;
		f32 deltaTime = 0;
		clock_t previousTime = {};
		FStyleSet* rootStyle = nullptr;

		RHI::DrawListContext drawList{};
		CE::JobManager* jobManager = nullptr;
		CE::JobContext* jobContext = nullptr;
    };
    
} // namespace CE
