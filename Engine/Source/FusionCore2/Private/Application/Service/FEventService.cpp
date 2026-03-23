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

            /*Ref<FSurface> curFocus = curFocusSurface.Lock();
            Ref<FSurface> focus = focusSurface.Lock();

            if (focus != curFocus) // Focused surface changed
            {
	            if (curFocus != nullptr)
	            {
                    curFocus->DispatchSurfaceUnfocusEvent();
	            }

                if (focus != nullptr)
                {
                    Vec2 screenMousePos = InputManager::Get().GetGlobalMousePosition();
                    Vec2 surfaceMousePos = focus->ScreenToSurfacePoint(screenMousePos);

                    focus->DispatchSurfaceFocusEvent();
                }
            }

            if (Ref<FSurface> surface = focus)
            {
                Vec2 screenMousePos = InputManager::Get().GetGlobalMousePosition();
                Vec2 surfaceMousePos = surface->ScreenToSurfacePoint(screenMousePos);

                FWidget* hitResult = surface->HitTestWidget(surfaceMousePos);

                if (hitResult)
                {
                    CE_LOG(Info, All, "Hit Result: {} [{}]", hitResult->GetName(), hitResult->GetClass()->GetName().GetLastComponent());
                }
            }

            this->curFocusSurface = this->focusSurface;*/
        }
    }

    void FEventService::FocusSurface(FSurface* surface)
    {
        if (focusSurface == surface)
            return;

        focusSurface = surface;
    }

} // namespace CE

