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

        void FocusSurface(FSurface* surface);

        Vec2 GetScreenMousePos() const { return screenMousePos; }
        Vec2 GetPrevScreenMousePos() const { return prevScreenMousePos; }

        Vec2 GetMouseWheelDelta() const { return wheelDelta; }

    protected:

        WeakRef<FSurface> curFocusSurface;
        WeakRef<FSurface> focusSurface;

        Vec2 screenMousePos;
        Vec2 prevScreenMousePos;
        Vec2 wheelDelta;

        b8 isFirstTick = true;
    };
    
} // namespace CE

#include "FEventService.rtti.h"
