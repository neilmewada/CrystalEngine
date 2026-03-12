#pragma once

namespace CE
{
    CLASS(Abstract)
    class FUSIONCORE_API FRenderService : public FService
    {
        CE_CLASS(FRenderService, FService)
    protected:

        FRenderService();
        
    public:

        // - Lifecycle -

        void OnStart() override;

        void TickService(FServiceTickPhase tickPhase) override;

        // - Rendering -

        virtual void MarkFrameGraphDirty() = 0;

        virtual void RenderPrepare() = 0;

        virtual bool BeginRender() = 0;
        virtual void EndRender() = 0;

        virtual int GetCurrentFrameIndex() = 0;

    protected:

        void UpdateDrawListMask(RHI::DrawListMask& drawListMask);


    };
    
} // namespace CE

#include "FRenderService.rtti.h"
