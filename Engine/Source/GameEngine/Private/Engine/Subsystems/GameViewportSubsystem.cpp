#include "GameEngine.h"

namespace CE
{

	GameViewportSubsystem::GameViewportSubsystem()
	{
		
	}

	GameViewportSubsystem::~GameViewportSubsystem()
	{

	}

	void GameViewportSubsystem::Initialize()
	{
		Super::Initialize();
	}

	void GameViewportSubsystem::PostInitialize()
	{
		Super::PostInitialize();

		FusionApplication* app = FusionApplication::TryGet();
		auto mainWindow = PlatformApplication::Get()->GetMainWindow();

		SceneSubsystem* sceneSubsystem = gEngine->GetSubsystem<SceneSubsystem>();
		CE::Scene* activeScene = sceneSubsystem->GetActiveScene();

		if (mainWindow && app)
		{
			auto rootContext = app->GetRootContext();

			FNativeContext* gameWindowContext = FNativeContext::Create(mainWindow, "GameWindow", rootContext);
			rootContext->AddChildContext(gameWindowContext);

			FAssignNewOwned(FGameWindow, gameWindow, gameWindowContext);
			gameWindowContext->SetOwningWidget(gameWindow);

			mainWindow->SetResizable(true);
			mainWindow->Show();

			gameWindow->GetContext()->SetClearScreen(false);
		}
	}

	void GameViewportSubsystem::PreShutdown()
	{
		Super::PreShutdown();
	}

	void GameViewportSubsystem::Shutdown()
	{
		Super::Shutdown();
	}

	f32 GameViewportSubsystem::GetTickPriority() const
	{
		return 3;
	}

	void GameViewportSubsystem::SetScene(CE::Scene* scene)
	{
		this->scene = scene;
		gameWindow->SetScene(scene->GetRpiScene());
		gEngine->GetSubsystem<RendererSubsystem>()->RebuildFrameGraph();

		gEngine->AddRenderViewport(gameWindow);
	}

	void GameViewportSubsystem::Tick(float deltaTime)
	{
		Super::Tick(deltaTime);

		auto mainWindow = PlatformApplication::Get()->GetMainWindow();

		if (scene && mainWindow)
		{
			if (RPI::Scene* rpiScene = scene->GetRpiScene())
			{
				rpiScene->SetPrimaryViewportSize(mainWindow->GetDrawableWindowSize());
			}
		}
	}
} // namespace CE
