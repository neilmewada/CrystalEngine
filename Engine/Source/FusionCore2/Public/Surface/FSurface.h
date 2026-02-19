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
        

    public: // - Public API -

        void GetDrawListMask(DrawListMask& drawListMask);

		void AddChildSurface(Ref<FSurface> childSurface);

        bool IsNativeSurface();

        void SetOwningWidget(Ref<FWidget> widget);

		Ref<FWidget> GetOwningWidget() const { return owningWidget; }


    public: // - Layout - 

		void AddPendingLayoutRoot(Ref<FWidget> layoutRoot);

    public: // - Lifecycle -

        virtual void Initialize() = 0;

        virtual void Shutdown() = 0;

        virtual void TickSurface(f32 deltaTime);

    protected:

        RHI::DrawListTag drawListTag = 0;
		RHI::ScopeId scopeId;

		HashSet<Uuid> pendingLayoutRootIds;
		Array<Ref<FWidget>> pendingLayoutRoots;

        Array<Ref<FSurface>> childrenSurfaces;
        
		WeakRef<FSurface> parentSurface;

		FIELD()
		Ref<FWidget> owningWidget;
    };
    
} // namespace CE

#include "FSurface.rtti.h"
