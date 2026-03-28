#include "FusionCore.h"

namespace CE
{
	static Ref<FApplication> gApplicationInstance;
	static WeakRef<FApplication> gApplicationInstanceRef;

    FApplication::FApplication()
    {
        defaultStyleSet = CreateDefaultSubobject<FStyleSet>("DefaultStyleSet");
        defaultStyleSet->InitDefault();
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

        Array<SubClass<FService>> serviceClasses = {
            GetStaticClass<FEventService>(),
            GetStaticClass<FAnimationService>(),
            GetStaticClass<FImageService>(),
        };

        serviceClasses.AddRange(initInfo.externalServices);

        for (const SubClass<FService>& serviceClass : serviceClasses)
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

        for (int i = destructionQueue.GetSize() - 1; i >= 0; i--)
        {
            destructionQueue[i].frameCounter += 1;

            if (destructionQueue[i].frameCounter > RHI::Limits::MaxSwapChainImageCount)
            {
                if (destructionQueue[i].itemToDestroy)
					destructionQueue[i].itemToDestroy->BeginDestroy();
                destructionQueue.RemoveAt(i);
            }
        }

        if (IsEngineRequestingExit())
            return;

		if (!exposed)
        {
			InvokeServiceTick(FServiceTickPhase::PumpPlatformEvents);
        }

        if (IsEngineRequestingExit())
            return;

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

        FTransitionManager::Get().Shutdown();
    }

    void FApplication::Shutdown()
    {
        for (int i = destructionQueue.GetSize() - 1; i >= 0; i--)
        {
            if (destructionQueue[i].itemToDestroy)
                destructionQueue[i].itemToDestroy->BeginDestroy();
        }
        destructionQueue.Clear();
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

    void FApplication::QueueDestroy(Ref<Object> object)
    {
        destructionQueue.Add({
            object,
            0
        });
    }

    void FApplication::InvokeServiceTick(FServiceTickPhase tickPhase)
    {
        for (Ref<FService> service : services)
        {
            service->TickService(tickPhase);
		}
    }
} // namespace CE

