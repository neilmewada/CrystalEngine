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

        virtual void Initialize() = 0;

		virtual void Shutdown() = 0;

        void GetDrawListMask(DrawListMask& drawListMask);

		void AddChildSurface(Ref<FSurface> childSurface);

        bool IsNativeSurface();

		void SetOwningWidget(Ref<FWidget> widget) { owningWidget = widget; }

		Ref<FWidget> GetOwningWidget() const { return owningWidget; }

    protected:

        RHI::DrawListTag drawListTag = 0;
		RHI::ScopeId scopeId;

        Array<Ref<FSurface>> childrenSurfaces;
        
		WeakRef<FSurface> parentSurface;

		FIELD()
		Ref<FWidget> owningWidget;
    };
    
} // namespace CE

#include "FSurface.rtti.h"
