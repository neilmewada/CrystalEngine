#include "FusionCore.h"

namespace CE
{

    FEventService::FEventService()
    {

    }

    void FEventService::TickService(FServiceTickPhase tickPhase)
    {
        Ref<FApplication> application = this->application.Lock();
        if (!application)
            return;

        if (tickPhase == FServiceTickPhase::PumpPlatformEvents)
        {
            PlatformApplication::Get()->Tick();
            InputManager::Get().Tick();

            prevScreenMousePos = screenMousePos;
            screenMousePos = InputManager::GetGlobalMousePosition();
            wheelDelta = InputManager::GetMouseWheelDelta();
            
        	if (isFirstTick)
            {
                isFirstTick = false;
                prevScreenMousePos = screenMousePos;
            }
		}
        else if (tickPhase == FServiceTickPhase::DispatchInput)
        {
            Ref<FSurface> curFocus = curFocusSurface.Lock();
            Ref<FSurface> focus = focusSurface.Lock();

            if (focus != curFocus)
            {
                if (curFocus) curFocus->DispatchSurfaceUnfocusEvent();
                if (focus)    focus->DispatchSurfaceFocusEvent();
            }

            if (focus)
            {
                focus->DispatchMouseEvents();  // surface resolves coords + runs state machine
                focus->DispatchKeyEvents();    // routes to curFocusedWidget
            }

            curFocusSurface = focusSurface;
        }
    }

    void FEventService::FocusSurface(FSurface* surface)
    {
        if (focusSurface == surface)
            return;

        focusSurface = surface;
    }

} // namespace CE

