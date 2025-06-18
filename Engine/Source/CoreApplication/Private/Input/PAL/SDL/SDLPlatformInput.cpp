#include "CoreApplication.h"

#include "SDL.h"

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

        if (event->type == SDL_WINDOWEVENT)
        {
	        switch (event->window.event)
	        {
	        case SDL_WINDOWEVENT_FOCUS_GAINED:
                focusGainedWindows.Add(event->window.windowID);
                break;
	        case SDL_WINDOWEVENT_FOCUS_LOST:
                focusLostWindows.Add(event->window.windowID);
                break;
	        }
        }
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
        mouseDelta = Vec2i(0, 0);

        SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
        u32 mouseBtnMask = SDL_GetGlobalMouseState(&globalMousePosition.x, &globalMousePosition.y);

        // TODO: Fix keyStatesDelayed: It is way too fast if FPS is high
        
        switch (event->type)
        {
        case SDL_KEYDOWN:
            windowId = event->key.windowID;
            if (keyStates[(KeyCode)event->key.keysym.sym])
            {
                keyStatesDelayed[(KeyCode)event->key.keysym.sym] = true;
            }
            else
            {
                stateChangesThisTick[(KeyCode)event->key.keysym.sym] = true;
            }
            keyStates[(KeyCode)event->key.keysym.sym] = true;
            modifierStates = (KeyModifier)event->key.keysym.mod;
            break;
        case SDL_KEYUP:
            windowId = event->key.windowID;
            if (keyStates[(KeyCode)event->key.keysym.sym])
            {
                stateChangesThisTick[(KeyCode)event->key.keysym.sym] = false;
            }
            keyStates[(KeyCode)event->key.keysym.sym] = false;
            keyStatesDelayed[(KeyCode)event->key.keysym.sym] = false;
            modifierStates = (KeyModifier)event->key.keysym.mod;
            break;
        case SDL_MOUSEBUTTONDOWN:
            windowId = event->button.windowID;
            if (mouseButtonStates[(MouseButton)event->button.button] != event->button.clicks)
            {
                mouseButtonStateChanges[(MouseButton)event->button.button] = event->button.clicks;
            }
            mouseButtonStates[(MouseButton)event->button.button] = event->button.clicks;
            break;
        case SDL_MOUSEBUTTONUP:
            windowId = event->button.windowID;
            if (mouseButtonStates[(MouseButton)event->button.button] != 0)
            {
                mouseButtonStateChanges[(MouseButton)event->button.button] = 0;
            }
            mouseButtonStates[(MouseButton)event->button.button] = 0;
            break;
        case SDL_MOUSEMOTION:
            windowId = event->motion.windowID;
            mouseDelta = Vec2i(event->motion.xrel, event->motion.yrel);
            break;
        case SDL_MOUSEWHEEL:
        {
            windowId = event->wheel.windowID;
#if PLATFORM_MAC
            f32 flipX = event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1;
            f32 flipY = event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? 1 : -1;
#else
            f32 flipX = -1.0f, flipY = 1.0f;
#endif

            wheelDelta = Vec2(event->wheel.preciseX * flipX, event->wheel.preciseY * flipY);
        }
            break;
        }

        if (mouseButtonStates[MouseButton::Left] != 0 && (mouseBtnMask & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0)
        {
            mouseButtonStateChanges[MouseButton::Left] = 0;
            mouseButtonStates[MouseButton::Left] = 0;
        }
    }

    void SDLPlatformInput::Tick()
    {
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
        input.mouseDelta = globalMousePosition - prevMousePosition;
        input.wheelDelta = wheelDelta;

        // Reset temp values
        prevMousePosition = globalMousePosition;
        wheelDelta = Vec2();

        stateChangesThisTick.Clear();
        mouseButtonStateChanges.Clear();
        focusGainedWindows.Clear();
        focusLostWindows.Clear();
    }

} // namespace CE
