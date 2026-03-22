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

        Ref<FSurface> GetFocusedSurface() const { return focusedSurface.Lock(); }

    protected:

        WeakRef<FWidget> capturedWidget;
        WeakRef<FWidget> inputLockedWidget;

        WeakRef<FSurface> focusedSurface;

    };
    
} // namespace CE

#include "FEventService.rtti.h"
