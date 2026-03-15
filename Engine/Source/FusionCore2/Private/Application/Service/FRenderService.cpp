#include "FusionCore.h"

namespace CE
{

    FRenderService::FRenderService()
    {

    }

    void FRenderService::OnStart()
    {
	    Super::OnStart();


    }

    void FRenderService::TickService(FServiceTickPhase tickPhase)
    {
        Ref<FApplication> application = this->application.Lock();
        if (!application)
            return;

        if (tickPhase == FServiceTickPhase::RenderPrepare)
        {
            RenderPrepare();

            for (int i = 0; i < application->GetSurfaceCount(); i++)
            {
                application->GetSurface(i)->RenderFrame(GetCurrentFrameIndex());
            }
        }
		else if (tickPhase == FServiceTickPhase::Render)
        {
            if (BeginRender())
            {
                for (int i = 0; i < application->GetSurfaceCount(); i++)
                {
                    application->GetSurface(i)->UpdateViewConstantBuffer(GetCurrentFrameIndex());
                }

                EndRender();
            }
        }
    }

    void FRenderService::UpdateDrawListMask(RHI::DrawListMask& drawListMask)
    {
        Ref<FApplication> application = this->application.Lock();
        if (!application)
            return;

		for (int i = 0; i < application->GetSurfaceCount(); i++)
        {
            if (Ref<FSurface> surface = application->GetSurface(i))
            {
                surface->GetDrawListMask(drawListMask);
            }
        }
    }

} // namespace CE

