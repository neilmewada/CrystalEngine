#include "FusionCore.h"

namespace CE
{

    FNativeSurface::FNativeSurface()
    {

    }

    void FNativeSurface::OnBeforeDestroy()
    {
	    Super::OnBeforeDestroy();

		Shutdown();
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
		scopeId = String::Format("NativeSurface_{}", platformWindow->GetWindowId());

        drawListTag = RPI::RPISystem::Get().GetDrawListTagRegistry()->AcquireTag(scopeId);

		PlatformApplication::Get()->AddMessageHandler(this);
    }

    void FNativeSurface::Shutdown()
    {
		PlatformApplication::Get()->RemoveMessageHandler(this);
    }
} // namespace CE

