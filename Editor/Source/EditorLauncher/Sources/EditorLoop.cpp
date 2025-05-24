#include "EditorLauncher.h"

EditorLoop gEditorLoop{};

extern const CE::String css;

void EditorLoop::PreInit(int argc, char** argv)
{
	// Setup before loading any module

	// Set Core Globals before loading Core
	gProjectName = "CrystalEngine";

	gDefaultWindowWidth = 1280;
	gDefaultWindowHeight = 720;

	gProgramArguments.Clear();
	for (int i = 0; i < argc; i++)
	{
		gProgramArguments.Add(argv[i]);
	}

	cxxopts::Options options{ "Crystal Editor", "Crystal Editor command line arguments." };

	options.add_options()
		("h,help", "Print this help info and exit")
		;

	options.allow_unrecognised_options();

	try
	{
		cxxopts::ParseResult result = options.parse(argc, argv);

		if (result["h"].as<bool>())
		{
			std::cout << options.help() << std::endl;
			exit(0);
		}

		auto positionalArgs = result.unmatched();

		for (const auto& arg : positionalArgs)
		{
			IO::Path path = IO::Path(arg);

			if (path.Exists() && path.GetExtension() == ProjectManager::GetProjectFileExtension())
			{
				projectPath = path;
				break;
			}
		}

		if (projectPath.IsEmpty())
		{
			// Open project browser
			gProjectPath = PlatformDirectories::GetLaunchDir();
		}
		else
		{
			// TODO: Open editor
			gProjectPath = projectPath.GetParentPath();
			gProjectName = projectPath.GetFileName().RemoveExtension().GetString();
		}
	}
	catch (const std::exception& exc)
	{
		CE_LOG(Error, All, "Failed to parse arguments: {}", exc.what());
		exit(-1);
	}

	// Initialize logging
	Logger::Initialize();
	Logger::SetConsoleLogLevel(LogLevel::Trace);
	Logger::SetFileDumpLogLevel(LogLevel::Trace);
}

void EditorLoop::LoadStartupCoreModules()
{
	// Load Startup Modules
	ModuleManager::Get().LoadModule("Core");
	ModuleManager::Get().LoadModule("CoreApplication");
	ModuleManager::Get().LoadModule("CoreInput");

	// Settings module
	ModuleManager::Get().LoadModule("CoreSettings");

	// Load Rendering modules
	ModuleManager::Get().LoadModule("CoreRHI");
#if PAL_TRAIT_VULKAN_SUPPORTED
	ModuleManager::Get().LoadModule("VulkanRHI");
#else
#	error Vulkan API is required
#endif
}

void EditorLoop::LoadCoreModules()
{
	// Load other Core modules
	ModuleManager::Get().LoadModule("CoreMedia");
	ModuleManager::Get().LoadModule("CoreMesh");
	ModuleManager::Get().LoadModule("CoreShader");
}

void EditorLoop::LoadEngineModules()
{
	ModuleManager::Get().LoadModule("CoreRPI");
	ModuleManager::Get().LoadModule("FusionCore");
	ModuleManager::Get().LoadModule("Fusion");

	ModuleManager::Get().LoadModule("Engine");
}

void EditorLoop::UnloadEngineModules()
{
	ModuleManager::Get().UnloadModule("Fusion");
	ModuleManager::Get().UnloadModule("FusionCore");

	ModuleManager::Get().UnloadModule("Engine");
	ModuleManager::Get().UnloadModule("CoreRPI");
}

void EditorLoop::LoadEditorModules()
{
	ModuleManager::Get().LoadModule("EditorCore");
	ModuleManager::Get().LoadModule("EditorWidgets");

	ModuleManager::Get().LoadModule("EditorEngine");

	ModuleManager::Get().LoadModule("CrystalEditor");

	Prefs::Get().LoadPrefsJson();

	ObjectEditorRegistry::Get().Init();
}

void EditorLoop::UnloadEditorModules()
{
	ObjectEditorRegistry::Get().Shutdown();

	Prefs::Get().SavePrefsJson();

	ModuleManager::Get().UnloadModule("CrystalEditor");

	ModuleManager::Get().UnloadModule("EditorEngine");

	ModuleManager::Get().UnloadModule("EditorWidgets");
	ModuleManager::Get().UnloadModule("EditorCore");
}


void EditorLoop::Init()
{
	// Load most important core modules for startup
	LoadStartupCoreModules();

	// Load application
	AppPreInit();
}

void EditorLoop::PostInit()
{
	LoadCoreModules();
	LoadEngineModules();
	LoadEditorModules();

	EditorCoreModule* editorCore = static_cast<EditorCoreModule*>(ModuleManager::Get().GetLoadedModule("EditorCore"));

	// Load Project
	LoadProject();

	AppInit();

	RHI::gDynamicRHI = new Vulkan::VulkanRHI();

	RHI::gDynamicRHI->Initialize();
	RHI::gDynamicRHI->PostInitialize();

	RHI::FrameSchedulerDescriptor desc{};
	desc.numFramesInFlight = 2;
	RHI::FrameScheduler::Create(desc);

	RPI::RPISystem::Get().Initialize();

	FusionApplication* fApp = FusionApplication::Get();

	Ref<CrystalEditorStyle> editorStyle = CreateObject<CrystalEditorStyle>(nullptr, "EditorStyle");
	editorCore->InitializeStyle(editorStyle.Get());

	gEngine->Initialize();

	gEngine->PostInitialize();

	Ref<AssetProcessor> assetProcessor = CrystalEditorModule::Get()->GetAssetProcessor();
	assetProcessor->Initialize();

	AssetManager* assetManager = AssetManager::Get();

	Ref<CE::Shader> standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
	Ref<CE::Shader> iblConvolutionShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/CubeMap/IBLConvolution");
	Ref<CE::Shader> textureGenShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/Utils/TextureGen");

	RPI::RPISystemInitInfo rpiInitInfo{};
	rpiInitInfo.standardShader = standardShader->GetShaderCollection();
	rpiInitInfo.iblConvolutionShader = iblConvolutionShader->GetShaderCollection();
	rpiInitInfo.textureGenShader = textureGenShader->GetShaderCollection();

	RPI::RPISystem::Get().PostInitialize(rpiInitInfo);

	auto rootContext = fApp->GetRootContext();

	if (!projectPath.Exists())
	{
		gEditorMode = EditorMode::ProjectBrowser;

		FNativeContext* projectBrowserContext = FNativeContext::Create(mainWindow, "ProjectBrowser", rootContext);
		rootContext->AddChildContext(projectBrowserContext);

		ProjectBrowser* projectBrowser = nullptr;

		FAssignNewOwned(ProjectBrowser, projectBrowser, projectBrowserContext);
		projectBrowserContext->SetOwningWidget(projectBrowser);

		projectBrowser->SetMaximizeButton(false);
		projectBrowser->SetMinimizeButton(true);

		mainWindow->SetResizable(false);
		mainWindow->Show();
	}
	else
	{
		gEditorMode = EditorMode::Default;

		auto editorHistoryFolder = projectPath.GetParentPath() / "Temp/EditorHistory";
		if (editorHistoryFolder.Exists()) // Clear editor history
		{
			IO::Path::RemoveRecursively(editorHistoryFolder);
			IO::Path::CreateDirectories(editorHistoryFolder);
		}

		FNativeContext* crystalEditorCtx = FNativeContext::Create(mainWindow, "CrystalEditor", rootContext);
		rootContext->AddChildContext(crystalEditorCtx);

		Ref<CrystalEditorWindow> crystalEditor = nullptr;

		FAssignNewOwned(CrystalEditorWindow, crystalEditor, crystalEditorCtx);
		crystalEditorCtx->SetOwningWidget(crystalEditor.Get());

		mainWindow->SetResizable(true);
		mainWindow->Show();
	}

	auto tickDelegate = MemberDelegate(&EditorLoop::ExposedTick, this);
	this->tickDelegateHandle = tickDelegate.GetHandle();
	app->AddTickHandler(tickDelegate);
}

void EditorLoop::InitStyles()
{

}

void EditorLoop::ExposedTick()
{
	auto app = PlatformApplication::Get();

	auto curTime = clock();
	deltaTime = (f32)(curTime - previousTime) / CLOCKS_PER_SEC;

	FusionApplication::Get()->SetExposed();

	gEngine->Tick(deltaTime);

	previousTime = curTime;
}

void EditorLoop::RunLoop()
{
	while (!IsEngineRequestingExit())
	{
		FusionApplication::Get()->ResetExposed();

		if (frameTimer >= 1.0f)
		{
			//CE_LOG(Info, All, "FPS: {}", frameCounter);

			frameTimer = 0;
			frameCounter = 0;
		}

		auto curTime = clock();
		deltaTime = (f32)(curTime - previousTime) / CLOCKS_PER_SEC;

		// App & Input Tick
		app->Tick();
		InputManager::Get().Tick();

		// Engine tick
		gEngine->Tick(deltaTime);

		previousTime = curTime;

		frameTimer += deltaTime;
		frameCounter++;
	}
}

void EditorLoop::PreShutdown()
{
	EditorCoreModule* editorCore = static_cast<EditorCoreModule*>(ModuleManager::Get().GetLoadedModule("EditorCore"));

	auto app = PlatformApplication::Get();
	app->RemoveTickHandler(tickDelegateHandle);

	// Destroy the project manager, which consequently saves it's Preferences to disk.
	if (auto projectManager = ProjectManager::TryGet())
	{
		projectManager->BeginDestroy();
	}

	// Save project & settings, and unload
	if (projectPath.Exists())
	{
		SaveSettings();
		UnloadSettings();
	}

	FusionApplication* fApp = FusionApplication::Get();

	fApp->PreShutdown();

	editorCore->ShutdownStyle();

	fApp->Shutdown();
	fApp->BeginDestroy();

	Ref<AssetProcessor> assetProcessor = CrystalEditorModule::Get()->GetAssetProcessor();
	assetProcessor->Shutdown();

	gEngine->PreShutdown();

	// fApp is Shutdown and destroyed by RendererSubsystem, no need to do it again here.
  
	RPI::RPISystem::Get().Shutdown();

	RHI::gDynamicRHI->PreShutdown();

	InputManager::Get().Shutdown(app);
	app->PreShutdown();

	gEngine->Shutdown();

	// Load editor modules
	UnloadEditorModules();

	// Unload engine modules
	UnloadEngineModules();

	AppPreShutdown();

	// Shutdown core widgets before RHI device
	ModuleManager::Get().UnloadModule("Fusion");
	ModuleManager::Get().UnloadModule("FusionCore");

	RHI::gDynamicRHI->Shutdown();

	// Unload modules

	ModuleManager::Get().UnloadModule("CoreShader");
	ModuleManager::Get().UnloadModule("CoreMedia");
	ModuleManager::Get().UnloadModule("CoreMesh");

#if PAL_TRAIT_VULKAN_SUPPORTED
	ModuleManager::Get().UnloadModule("VulkanRHI");
#endif
	ModuleManager::Get().UnloadModule("CoreRHI");
}

void EditorLoop::Shutdown()
{
	IO::Path exitLaunchProcess = gExitLaunchProcess;
	String exitLaunchArgs = gExitLaunchArguments;
	
	// Shutdown application
	AppShutdown();

	// Unload settings module
	ModuleManager::Get().UnloadModule("CoreSettings");

	// Unload most important modules at last
	ModuleManager::Get().UnloadModule("CoreInput");
	ModuleManager::Get().UnloadModule("CoreApplication");
	ModuleManager::Get().UnloadModule("Core");

	Logger::Shutdown();

	if (!exitLaunchProcess.IsEmpty())
	{
		PlatformProcess::LaunchProcess(exitLaunchProcess, exitLaunchArgs);
	}
}

void EditorLoop::LoadProject()
{
	if (!projectPath.IsEmpty())
	{
		bool success = ProjectManager::Get()->LoadProject(projectPath);
		if (!success)
		{
			PlatformProcess::LaunchProcess(PlatformDirectories::GetLaunchDir() / "EditorLauncher", "");
			exit(0);
		}
	}
}

void EditorLoop::AppPreInit()
{
	app = PlatformApplication::Get();
	app->Initialize();

	InputManager::Get().Initialize(app);
	
}

void EditorLoop::AppInit()
{
	gEngine->PreInit();
	bool isProjectBrowsingMode = false;

	gDefaultWindowWidth = 1366;
	gDefaultWindowHeight = 768;

	Vec2i screenSize = app->GetMainScreenSize();
	if (screenSize.x > 2560 && screenSize.y > 1440)
	{
		gDefaultWindowWidth = 2560 * FusionApplication::Get()->GetDefaultScalingFactor();
		gDefaultWindowHeight = 1440 * FusionApplication::Get()->GetDefaultScalingFactor();
	}

	PlatformWindowInfo windowInfo{};
	windowInfo.maximised = windowInfo.fullscreen = false;
	windowInfo.resizable = true;
	windowInfo.hidden = true;
#if PLATFORM_LINUX
	windowInfo.hidden = false; // Weird behaviour on linux: Window doesn't show when called Show() if hidden is true on initialization.
#endif
	windowInfo.windowFlags = PlatformWindowFlags::DestroyOnClose;
	if (projectPath.IsEmpty())
	{
		isProjectBrowsingMode = true;
		//windowInfo.windowFlags |= PlatformWindowFlags::Utility;
		f32 systemScaling = PlatformApplication::Get()->GetSystemDpiScaling();

#if PLATFORM_MAC
		gDefaultWindowWidth = 800;
		gDefaultWindowHeight = 650;
#else
		gDefaultWindowWidth = 800 * systemScaling * FusionApplication::Get()->GetDefaultScalingFactor();
		gDefaultWindowHeight = 650 * systemScaling * FusionApplication::Get()->GetDefaultScalingFactor();
#endif
	}

	mainWindow = app->InitMainWindow(MODULE_NAME, gDefaultWindowWidth, gDefaultWindowHeight, windowInfo);

	mainWindow->SetMinimumSize(isProjectBrowsingMode ? Vec2i(gDefaultWindowWidth, gDefaultWindowHeight) : Vec2i(1280, 720));
	mainWindow->SetBorderless(true);
}

void EditorLoop::AppPreShutdown()
{
	app->PreShutdown();
}

void EditorLoop::AppShutdown()
{
	app->Shutdown();
	
	delete app;
	app = nullptr;
}
