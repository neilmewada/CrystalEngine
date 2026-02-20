#include "CoreApplication.h"

#include <SDL3/SDL.h>

namespace CE
{

    SDLPlatformInput::SDLPlatformInput()
    {

    }

    SDLPlatformInput::~SDLPlatformInput()
    {

    }

    SDLPlatformInput* SDLPlatformInput::Create()
    {
        return new SDLPlatformInput();
    }

    void SDLPlatformInput::ProcessNativeEvent(void* nativeEvent)
    {
        if (nativeEvent == nullptr)
            return;

        SDL_Event* event = (SDL_Event*)nativeEvent;

        if (event->type == SDL_EVENT_WINDOW_FOCUS_GAINED)
            focusGainedWindows.Add(event->window.windowID);
        else if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST)
            focusLostWindows.Add(event->window.windowID);
    }

    void SDLPlatformInput::ProcessInputEvent(void* nativeEvent)
    {
        if (nativeEvent == nullptr)
            return;

        auto app = PlatformApplication::Get();
        if (app == nullptr || app->GetMainWindow() == nullptr)
			return;

        InputManager& input = InputManager::Get();
        SDL_Window* window = (SDL_Window*)app->GetMainWindow()->GetUnderlyingHandle();

        SDL_Event* event = (SDL_Event*)nativeEvent;

        SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
        u32 mouseBtnMask = SDL_GetGlobalMouseState(&globalMousePosition.x, &globalMousePosition.y);

        // TODO: Fix keyStatesDelayed: It is way too fast if FPS is high
        
        switch (event->type)
        {
        case SDL_EVENT_KEY_DOWN:
            windowId = event->key.windowID;
            if (keyStates[(KeyCode)event->key.key])
            {
                keyStatesDelayed[(KeyCode)event->key.key] = { .state = true, .lastEnabledTime = curTime };
            }
            else
            {
                stateChangesThisTick[(KeyCode)event->key.key] = true;
            }
            keyStates[(KeyCode)event->key.key] = true;
            modifierStates = (KeyModifier)event->key.mod;
            break;
        case SDL_EVENT_KEY_UP:
            windowId = event->key.windowID;
            if (keyStates[(KeyCode)event->key.key])
            {
                stateChangesThisTick[(KeyCode)event->key.key] = false;
            }
            keyStates[(KeyCode)event->key.key] = false;
            keyStatesDelayed[(KeyCode)event->key.key] = { .state = false, .lastEnabledTime = 0};
            modifierStates = (KeyModifier)event->key.mod;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            windowId = event->button.windowID;
            if (mouseButtonStates[(MouseButton)event->button.button] != event->button.clicks)
            {
                mouseButtonStateChanges[(MouseButton)event->button.button] = event->button.clicks;
            }
            mouseButtonStates[(MouseButton)event->button.button] = event->button.clicks;
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            windowId = event->button.windowID;
            if (mouseButtonStates[(MouseButton)event->button.button] != 0)
            {
                mouseButtonStateChanges[(MouseButton)event->button.button] = 0;
            }
            mouseButtonStates[(MouseButton)event->button.button] = 0;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            windowId = event->motion.windowID;
            break;
        case SDL_EVENT_MOUSE_WHEEL:
        {
            windowId = event->wheel.windowID;
#if PLATFORM_MAC
            f32 flipX = event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1;
            f32 flipY = event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? 1 : -1;
#else
            f32 flipX = -1.0f, flipY = 1.0f;
#endif

            wheelDelta = Vec2(event->wheel.x * flipX, event->wheel.y * flipY);
        }
            break;
        }

        if (mouseButtonStates[MouseButton::Left] != 0 && (mouseBtnMask & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) == 0)
        {
            mouseButtonStateChanges[MouseButton::Left] = 0;
            mouseButtonStates[MouseButton::Left] = 0;
        }
    }

    void SDLPlatformInput::Tick()
    {
        auto now = std::chrono::high_resolution_clock::now();
        curTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

        InputManager& input = InputManager::Get();
        auto app = PlatformApplication::Get();

        SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
        SDL_GetGlobalMouseState(&globalMousePosition.x, &globalMousePosition.y);

        input.stateChangesThisTick = stateChangesThisTick;
        input.keyStates = keyStates;
        input.modifierStates = modifierStates;
        input.keyStatesDelayed = keyStatesDelayed;
        input.mouseButtonStates = mouseButtonStates;
        input.mouseButtonStateChanges = mouseButtonStateChanges;
        input.windowId = windowId;
        input.mousePosition = mousePosition;
        input.globalMousePosition = globalMousePosition;
        input.mouseDelta = globalMousePosition - prevGlobalMousePosition;
        input.wheelDelta = wheelDelta;
        input.curTime = curTime;

        // Reset temp values
        prevGlobalMousePosition = globalMousePosition;
        wheelDelta = Vec2();

        stateChangesThisTick.Clear();
        mouseButtonStateChanges.Clear();
        focusGainedWindows.Clear();
        focusLostWindows.Clear();
    }

} // namespace CE
