#include "FusionCore.h"

namespace CE
{

    FNativeSurface::FNativeSurface()
    {

    }

    FNativeSurface* FNativeSurface::Create(PlatformWindow* window, const String& name, FSurface* parentSurface)
    {
		if (!FApplication::Get()->IsNativeSurfaceSupported())
        {
            return nullptr;
        }

        Object* outer = parentSurface;

		if (!outer)
        {
            outer = FApplication::Get();
        }
        if (!outer)
        {
            return nullptr;
        }

        FNativeSurface* nativeContext = CreateObject<FNativeSurface>(outer, FixObjectName(name));
		nativeContext->platformWindow = window;

        if (parentSurface)
        {
			parentSurface->AddChildSurface(nativeContext);
        }

		nativeContext->Initialize();

        return nativeContext;
    }

    void FNativeSurface::Initialize()
    {

    }

    void FNativeSurface::Shutdown()
    {

    }
} // namespace CE

