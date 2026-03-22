#pragma once

namespace CE
{
    class FSurface;

    CLASS()
    class FUSIONCORE_API FEventService : public FService
    {
        CE_CLASS(FEventService, FService)
    protected:

        FEventService();
        
    public:

        void TickService(FServiceTickPhase tickPhase) override;

    protected:

        WeakRef<FWidget> capturedWidget;
        WeakRef<FWidget> pressedWidget;
        WeakRef<FWidget> hoveredWidget;

    };
    
} // namespace CE

#include "FEventService.rtti.h"
