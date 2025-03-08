#include "GameLauncher.h"

GameLoop gGameLoop{};

void GameLoop::PreInit(int argc, char** argv)
{
	// Setup before loading anything

	gProjectPath = PlatformDirectories::GetLaunchDir();
	String folderName = gProjectPath.GetFileName().GetString();

	if (folderName != "Debug" && folderName != "Development" && folderName != "Profile" && folderName != "Release")
	{
		// Running inside final runtime build
		gProjectName = folderName;
	}
	else
	{
		// Running from build output folder (in sandbox testing environment)
		gProjectName = "CrystalEngine.Standalone";
	}

	if (gDefaultWindowWidth == 0)
		gDefaultWindowWidth = 1280;
	if (gDefaultWindowHeight == 0)
		gDefaultWindowHeight = 720;

	// Initialize logging
	Logger::Initialize();
	Logger::SetConsoleLogLevel(LogLevel::Trace);
	Logger::SetFileDumpLogLevel(LogLevel::Trace);

	gProgramArguments.Clear();
	for (int i = 0; i < argc; i++)
	{
		gProgramArguments.Add(argv[i]);
	}

	cxxopts::Options options = cxxopts::Options(gProjectName.ToStdString(), (gProjectName + " command line arguments.").ToStdString());

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
			return;
		}


	}
	catch (std::exception exc)
	{
		CE_LOG(Error, All, "Failed to parse arguments: {}", exc.what());
		exit(-1);
	}
}

void GameLoop::LoadStartupCoreModules()
{
	// Load Startup Modules
	ModuleManager::Get().LoadModule("Core");
	ModuleManager::Get().LoadModule("CoreApplication");

	ModuleManager::Get().LoadModule("CoreSettings");
	ModuleManager::Get().LoadModule("CoreShader");

	// Load Rendering modules
	ModuleManager::Get().LoadModule("CoreRHI");
#if PAL_TRAIT_VULKAN_SUPPORTED
	ModuleManager::Get().LoadModule("VulkanRHI");
#else
#	error Vulkan API is required but not supported by current configuration
#endif
}

void GameLoop::LoadCoreModules()
{
	// Load other Core modules
	ModuleManager::Get().LoadModule("CoreMedia");
	ModuleManager::Get().LoadModule("CoreMesh");
}

void GameLoop::LoadEngineModules()
{
	ModuleManager::Get().LoadModule("CoreRPI");
	ModuleManager::Get().LoadModule("FusionCore");
	ModuleManager::Get().LoadModule("Fusion");

	ModuleManager::Get().LoadModule("Engine");
	ModuleManager::Get().LoadModule("GameEngine");
	// TODO: Sandbox
	ModuleManager::Get().LoadModule("Sandbox");
}

void GameLoop::UnloadEngineModules()
{
	ModuleManager::Get().UnloadModule("Fusion");
	ModuleManager::Get().UnloadModule("FusionCore");

	// TODO: Sandbox
	ModuleManager::Get().UnloadModule("Sandbox");

	ModuleManager::Get().UnloadModule("GameEngine");
	ModuleManager::Get().UnloadModule("Engine");

	// CoreRPI should always shutdown after the engine modules!
	ModuleManager::Get().UnloadModule("CoreRPI");
}

void GameLoop::Init()
{
	// Load most important core modules for startup
	LoadStartupCoreModules();

	// Load application
	AppPreInit();
}

void GameLoop::PostInit()
{
	LoadCoreModules();
	LoadEngineModules();

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

	// Initialize engine & it's subsystems, and set game viewport
	gEngine->Initialize();

	// Post initialize
	gEngine->PostInitialize();

	AssetManager* assetManager = AssetManager::Get();

	Ref<CE::Shader> standardShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
	Ref<CE::Shader> iblConvolutionShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/CubeMap/IBLConvolution");
	Ref<CE::Shader> textureGenShader = assetManager->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/Utils/TextureGen");

	RPI::RPISystemInitInfo rpiInitInfo{};
	rpiInitInfo.standardShader = standardShader->GetShaderCollection();
	rpiInitInfo.iblConvolutionShader = iblConvolutionShader->GetShaderCollection();
	rpiInitInfo.textureGenShader = textureGenShader->GetShaderCollection();

	RPI::RPISystem::Get().PostInitialize(rpiInitInfo);

	auto tickDelegate = MemberDelegate(&GameLoop::ExposedTick, this);
	this->tickDelegateHandle = tickDelegate.GetHandle();
	app->AddTickHandler(tickDelegate);

	SetupTestScene();
}

void GameLoop::ExposedTick()
{
	auto app = PlatformApplication::Get();

	auto curTime = clock();
	deltaTime = (f32)(curTime - previousTime) / CLOCKS_PER_SEC;

	FusionApplication::Get()->SetExposed();

	gEngine->Tick(deltaTime);

	previousTime = curTime;
}

void GameLoop::RunLoop()
{
	previousTime = clock();
	auto fApp = FusionApplication::Get();

	while (!IsEngineRequestingExit())
	{
		if (fApp->IsExposed())
		{
			fApp->ResetExposed();
			previousTime = clock(); // Reset previous time when window becomes unexposed
		}

		auto curTime = clock();
		deltaTime = (f32)(curTime - previousTime) / CLOCKS_PER_SEC;

		// App & Input Tick
		app->Tick();
		InputManager::Get().Tick();

		// Engine tick
		gEngine->Tick(deltaTime);

		previousTime = curTime;
	}
}

void GameLoop::PreShutdown()
{
	auto app = PlatformApplication::Get();
	app->RemoveTickHandler(tickDelegateHandle);

	// Save project & settings, and unload
	if (projectPath.Exists())
	{
		SaveSettings();
		UnloadSettings();
	}

	FusionApplication* fApp = FusionApplication::Get();

	fApp->PreShutdown();

	fApp->Shutdown();
	fApp->BeginDestroy();

	gEngine->PreShutdown();

	// fApp is Shutdown and destroyed by RendererSubsystem, no need to do it again here.

	RPI::RPISystem::Get().Shutdown();

	RHI::gDynamicRHI->PreShutdown();

	InputManager::Get().Shutdown(app);
	app->PreShutdown();

	gEngine->Shutdown();

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

void GameLoop::Shutdown()
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

void GameLoop::LoadProject()
{
	// Load project (aka Setings)
	Ref<Bundle> settingsBundle = GetSettingsBundle();

	Ref<ProjectSettings> projectSettings = GetSettings<ProjectSettings>();
	if (projectSettings != nullptr)
	{
		gProjectName = projectSettings->projectName;
	}
}

void GameLoop::UnloadProject()
{
	
}

void GameLoop::InitStyles()
{
	
}

void GameLoop::AppPreInit()
{
	app = PlatformApplication::Get();
}

void GameLoop::AppInit()
{
	gEngine->PreInit();
	app->Initialize();

	InputManager::Get().Initialize(app);

	String windowTitle = gProjectName;
	Ref<ProjectSettings> projectSettings = GetSettings<ProjectSettings>();
	if (projectSettings != nullptr)
	{
		windowTitle += " - v" + projectSettings->projectVersion;
	}

	mainWindow = app->InitMainWindow(windowTitle, gDefaultWindowWidth, gDefaultWindowHeight, false, false);
	mainWindow->Show();
}

void GameLoop::AppPreShutdown()
{
	app->PreShutdown();
}

void GameLoop::AppShutdown()
{
	app->Shutdown();

	delete app;
	app = nullptr;
}


