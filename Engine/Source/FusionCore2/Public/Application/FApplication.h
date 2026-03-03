#pragma once

namespace CE
{
    class FStyleSet;
    class FService;
    class FSurface;
    class FNativeSurface;

    struct FApplicationInitInfo
    {
		Array<SubClass<FService>> services;
	};

    CLASS(Config = Engine)
    class FUSIONCORE_API FApplication : public CE::Object
    {
        CE_CLASS(FApplication, CE::Object)
    protected:

        FApplication();

		void OnBeginDestroy() override;

    public:

		static FApplication* GetOrCreate(SubClass<FApplication> clazz = Self::StaticClass());

        static FApplication* Get();

        // - Limits / Capabilities -

		bool IsDockingSupported() const;

		bool IsNativeSurfaceSupported() const;

		// - Lifecycle -

		bool Initialize(const FApplicationInitInfo& initInfo);

        void Tick(f32 deltaTime, bool exposed = false);

		void PreShutdown();

		void Shutdown();

		// - Getters & Setters -

		f32 GetDeltaTime() const { return deltaTime; }

		bool HasService(ClassType* serviceClass) const;

        template<typename TService> requires TIsBaseClassOf<FService, TService>::Value
        bool HasService() const
        {
			return HasService(TService::Type());
        }

		Ref<FService> GetService(ClassType* serviceClass) const;

		template<typename TService> requires TIsBaseClassOf<FService, TService>::Value
        Ref<TService> GetService() const
        {
			return CastTo<TService>(GetService(TService::Type()));
        }

		u32 GetSurfaceCount() const { return surfaces.GetSize(); }

		Ref<FSurface> GetSurface(u32 index) const { return (index < surfaces.GetSize()) ? surfaces[index] : nullptr; }

        void AddSurface(Ref<FSurface> surface);
        void RemoveSurface(Ref<FSurface> surface);
        
    protected:

		void InvokeServiceTick(FServiceTickPhase tickPhase);

		f32 deltaTime = 0;

        Array<Ref<FSurface>> surfaces;
        
		Array<Ref<FService>> services;

    private:

        friend class FNativeSurface;
    };
    
} // namespace CE

#include "FApplication.rtti.h"
