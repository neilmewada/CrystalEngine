#include "FusionCore.h"

namespace CE
{
	static Ref<FApplication> gApplicationInstance;
	static WeakRef<FApplication> gApplicationInstanceRef;

    FApplication::FApplication()
    {
        defaultStyleSet = CreateDefaultSubobject<FStyleSet>("DefaultStyleSet");
    }

    void FApplication::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

		gApplicationInstance = nullptr;
    }

    FApplication* FApplication::GetOrCreate(SubClass<FApplication> clazz)
    {
		if (clazz == nullptr)
        {
            clazz = Self::StaticClass();
        }

        if (gApplicationInstanceRef.IsValid())
        {
            return gApplicationInstanceRef.Get();
        }

        if (gApplicationInstance.IsValid())
        {
			return gApplicationInstance.Get();
        }

		gApplicationInstance = CreateObject<FApplication>(GetTransient(MODULE_NAME), "FusionApplication", OF_NoFlags, clazz);
		gApplicationInstanceRef = gApplicationInstance;

		return gApplicationInstance.Get();
    }

    FApplication* FApplication::Get()
    {
		return gApplicationInstanceRef.Get();
    }

    bool FApplication::IsDockingSupported() const
    {
#if PAL_TRAIT_BUILD_EDITOR
        return true;
#else
		return false;
#endif
    }

    bool FApplication::IsNativeSurfaceSupported() const
    {
#if PAL_TRAIT_BUILD_EDITOR
        return true;
#else
        return false;
#endif
    }

    bool FApplication::Initialize(const FApplicationInitInfo& initInfo)
    {
        ZoneScoped;

        defaultDpiScale = PlatformApplication::Get()->GetSystemDpiScaling();

		bool hasRenderService = false;

        for (const SubClass<FService>& serviceClass : initInfo.services)
        {
            if (serviceClass == nullptr || !serviceClass->CanBeInstantiated())
                continue;
            
            Ref<FService> service = CreateObject<FService>(this, serviceClass->GetName().GetLastComponent(), OF_NoFlags, serviceClass);
            if (service == nullptr)
                continue;

            service->application = this;

			if (service->IsOfType<FRenderService>())
            {
                hasRenderService = true;
            }

            services.Add(service);
		}

        if (!hasRenderService)
        {
            return false;
        }

        for (Ref<FService> service : services)
        {
			service->OnStart();
        }

        return true;
    }

    void FApplication::Tick(f32 deltaTime, bool exposed)
    {
        ZoneScoped;

		this->deltaTime = deltaTime;

		if (!exposed)
        {
			InvokeServiceTick(FServiceTickPhase::PumpPlatformEvents);
        }

		InvokeServiceTick(FServiceTickPhase::DispatchInput);

		InvokeServiceTick(FServiceTickPhase::PreUpdateSurfaces);

        for (int i = 0; i < surfaces.GetSize(); i++)
        {
			surfaces[i]->TickSurface(deltaTime);
        }

		InvokeServiceTick(FServiceTickPhase::PostUpdateSurfaces);

		InvokeServiceTick(FServiceTickPhase::RenderPrepare);

		InvokeServiceTick(FServiceTickPhase::Render);

		InvokeServiceTick(FServiceTickPhase::PostRender);
    }

    void FApplication::PreShutdown()
    {
        ZoneScoped;

        for (Ref<FService> service : services)
        {
			service->OnShutdown();
        }
        services.Clear();
    }

    void FApplication::Shutdown()
    {
    }

    bool FApplication::HasService(ClassType* serviceClass) const
    {
        for (Ref<FService> service : services)
        {
            if (service->GetClass()->IsSubclassOf(serviceClass))
                return true;
		}

		return false;
    }

    Ref<FService> FApplication::GetService(ClassType* serviceClass) const
    {
        for (Ref<FService> service : services)
        {
            if (service->GetClass()->IsSubclassOf(serviceClass))
                return service;
        }
		return nullptr;
    }

    void FApplication::AddSurface(Ref<FSurface> surface)
    {
		if (!surface || surfaces.Exists(surface))
            return;

		surfaces.Add(surface);
    }

    void FApplication::RemoveSurface(Ref<FSurface> surface)
    {
		if (surfaces.Remove(surface))
		{
			GetService<FRenderService>()->MarkFrameGraphDirty();
		}
    }

    void FApplication::InvokeServiceTick(FServiceTickPhase tickPhase)
    {
        for (Ref<FService> service : services)
        {
            service->TickService(tickPhase);
		}
    }
} // namespace CE

