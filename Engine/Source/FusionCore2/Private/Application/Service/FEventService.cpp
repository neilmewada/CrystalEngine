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
		}
        else if (tickPhase == FServiceTickPhase::DispatchInput)
        {
            if (Ref<FSurface> surface = mouseFocusedSurface.Lock())
            {
                Vec2 screenMousePos = InputManager::Get().GetGlobalMousePosition();
                Vec2 surfaceMousePos = surface->ScreenToSurfacePoint(screenMousePos);
                surfaceMousePos /= surface->GetDpiScale();

                FWidget* hitResult = surface->HitTestWidget(surfaceMousePos);

                if (hitResult)
                {
                    CE_LOG(Info, All, "Hit Result: {} [{}]", hitResult->GetName(), hitResult->GetClass()->GetName().GetLastComponent());
                }
            }
        }
    }

} // namespace CE

