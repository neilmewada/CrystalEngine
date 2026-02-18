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

        void TickService(FServiceTickPhase tickPhase) override;

        // - Renderer Service -

        virtual void MarkFrameGraphDirty() = 0;

        virtual void RenderPrepare() = 0;

		virtual void Render() = 0;

    protected:

        void UpdateDrawListMask(DrawListMask& drawListMask);

    };
    
} // namespace CE

#include "FRenderService.rtti.h"
