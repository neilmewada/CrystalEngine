#pragma once

namespace CE
{
    class FWidget;

    CLASS(Abstract)
    class FUSIONCORE_API FSurface : public CE::Object
    {
        CE_CLASS(FSurface, CE::Object)
    protected:

        FSurface();
        
    public:

		void GetDrawListMask(DrawListMask& drawListMask);

        virtual void Initialize() = 0;

		virtual void Shutdown() = 0;

		void AddChildSurface(Ref<FSurface> childSurface);

        bool IsNativeSurface();

    protected:

        RHI::DrawListTag drawListTag = 0;
		RHI::ScopeId scopeId;

        Array<Ref<FSurface>> childrenSurfaces;
        
		WeakRef<FSurface> parentSurface;
    };
    
} // namespace CE

#include "FSurface.rtti.h"
