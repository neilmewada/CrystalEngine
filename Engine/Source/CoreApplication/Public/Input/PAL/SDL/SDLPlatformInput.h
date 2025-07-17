#pragma once

#include "Input/PAL/Common/PlatformInput.h"

#include <chrono>

namespace CE
{

    class COREAPPLICATION_API SDLPlatformInput : public PlatformInput
    {
    public:
        SDLPlatformInput();
        ~SDLPlatformInput();

        static SDLPlatformInput* Create();

        virtual void ProcessInputEvent(void* nativeEvent) override;

        virtual void Tick() override;

        void ProcessNativeEvent(void* nativeEvent) override;

    private:

        std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
        u64 curTime = 0;

        u64 windowId = 0;
        Vec2i globalMousePosition{};
        Vec2i mousePosition{};
        Vec2i prevMousePosition{};
        Vec2i mouseDelta{};
        Vec2 wheelDelta{};

        Array<u64> focusGainedWindows{};
        Array<u64> focusLostWindows{};

        HashMap<KeyCode, bool> keyStates{};
        HashMap<KeyCode, Internal::KeyStateDelayed> keyStatesDelayed{};
        HashMap<MouseButton, int> mouseButtonStates{};

        // Per-Tick changes
        HashMap<KeyCode, bool> stateChangesThisTick{};
        HashMap<MouseButton, int> mouseButtonStateChanges{};
        
        KeyModifier modifierStates{};
    };

    typedef SDLPlatformInput PlatformInputImpl;
    
} // namespace CE
