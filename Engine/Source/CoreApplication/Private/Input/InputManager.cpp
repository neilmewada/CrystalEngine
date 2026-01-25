#include "CoreApplication.h"
#include "Input/PAL/SDL/SDLPlatformInput.h"

namespace CE
{

	InputManager& InputManager::Get()
	{
		static InputManager instance{};
		return instance;
	}

	void InputManager::Initialize(PlatformApplication* app)
	{
		platformInput = PlatformInputImpl::Create();

		app->AddMessageHandler(this);
	}

	void InputManager::Shutdown(PlatformApplication* app)
	{
		app->RemoveMessageHandler(this);

		delete platformInput; platformInput = nullptr;
	}

	void InputManager::Tick()
	{
		platformInput->Tick();
	}

	bool InputManager::IsKeyHeldDelayed(KeyCode key)
	{
		if (!Get().keyStatesDelayed.KeyExists(key))
		{
			return false;
		}

		if (Get().curTime - Get().keyStatesDelayed[key].lastEnabledTime < 25)
		{
			return false;
		}

		Get().keyStatesDelayed[key].lastEnabledTime = Get().curTime;
		return Get().keyStatesDelayed[key].state;
	}

	void InputManager::ProcessInputEvents(void* nativeEvent)
	{
		platformInput->ProcessInputEvent(nativeEvent);
	}

	void InputManager::ProcessNativeEvents(void* nativeEvent)
	{
		platformInput->ProcessNativeEvent(nativeEvent);
	}

} // namespace CE
