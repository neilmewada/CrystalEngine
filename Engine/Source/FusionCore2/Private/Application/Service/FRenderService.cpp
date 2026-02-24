#include "FusionCore.h"

namespace CE
{

    FRenderService::FRenderService()
    {

    }

    void FRenderService::TickService(FServiceTickPhase tickPhase)
    {
        if (tickPhase == FServiceTickPhase::RenderPrepare)
        {
            RenderPrepare();
        }
		else if (tickPhase == FServiceTickPhase::Render)
        {
            RenderFrame();
        }
    }

    void FRenderService::UpdateDrawListMask(RHI::DrawListMask& drawListMask)
    {
		for (int i = 0; i < application->GetSurfaceCount(); i++)
        {
            if (Ref<FSurface> surface = application->GetSurface(i))
            {
                surface->GetDrawListMask(drawListMask);
            }
        }
    }

} // namespace CE

