
#include "Core.h"

namespace CE
{
    // Module Class
    class CoreModule : public Module
    {
    public:
        void StartupModule() override;
        void ShutdownModule() override;

        void RegisterTypes() override;

    private:
        DelegateHandle onBeforeModuleUnloadHandle = 0;
		DelegateHandle settingsBaseOnClassRegistered = 0;
		DelegateHandle settingsBaseOnClassDeregistered = 0;

        DelegateHandle prefsOnClassRegistered = 0;
        DelegateHandle prefsOnClassDeregistered = 0;
    };


    WeakRef<Bundle> gSettingsBundle = nullptr;
	ResourceManager* gResourceManager = nullptr;

    void CoreModule::StartupModule()
    {
        gMainThreadId = Thread::GetCurrentThreadId();

		gConfigCache = new ConfigCache();

#if PAL_TRAIT_BUILD_EDITOR
        if (gProjectPath.IsEmpty()) // Editor: gProjectPath should be set before loading Core
            gProjectPath = PlatformDirectories::GetEngineRootDir();
#endif

		// Load .ini configs into cache
		gConfigCache->LoadStartupConfigs();
        
        onBeforeModuleUnloadHandle = CoreDelegates::onBeforeModuleUnload.AddDelegateInstance(&TypeInfo::DeregisterTypesForModule);

		settingsBaseOnClassRegistered = CoreObjectDelegates::onClassRegistered.AddDelegateInstance(&Settings::OnClassRegistered);
		settingsBaseOnClassDeregistered = CoreObjectDelegates::onClassDeregistered.AddDelegateInstance(&Settings::OnClassDeregistered);

        prefsOnClassRegistered = CoreObjectDelegates::onClassRegistered.AddDelegateInstance(&Prefs::OnClassRegistered);
        prefsOnClassDeregistered = CoreObjectDelegates::onClassDeregistered.AddDelegateInstance(&Prefs::OnClassDeregistered);
    }

    void CoreModule::ShutdownModule()
    {
		if (gResourceManager != nullptr)
		{
			gResourceManager->BeginDestroy();
		}
        gResourceManager = nullptr;

		if (gSettingsBundle != nullptr)
		{
			gSettingsBundle->BeginDestroy();
		}
        gSettingsBundle = nullptr;

		if (gConfigCache != nullptr)
		{
			delete gConfigCache;
			gConfigCache = nullptr;
		}

        gProjectPath = "";

        CoreObjectDelegates::onClassDeregistered.RemoveDelegateInstance(prefsOnClassDeregistered);
        prefsOnClassDeregistered = 0;
        CoreObjectDelegates::onClassRegistered.RemoveDelegateInstance(prefsOnClassRegistered);
        prefsOnClassRegistered = 0;

		CoreObjectDelegates::onClassDeregistered.RemoveDelegateInstance(settingsBaseOnClassDeregistered);
		settingsBaseOnClassDeregistered = 0;
		CoreObjectDelegates::onClassRegistered.RemoveDelegateInstance(settingsBaseOnClassRegistered);
		settingsBaseOnClassRegistered = 0;

        CoreDelegates::onBeforeModuleUnload.RemoveDelegateInstance(onBeforeModuleUnloadHandle);
        onBeforeModuleUnloadHandle = 0;
    }

    void CoreModule::RegisterTypes()
    {
        // Register Data types
        CE_REGISTER_TYPES(b8, s8, s16, s32, s64,
            u8, u16, u32, u64, f32, f64, c8, c16,
            CE::String, CE::Uuid, CE::Name, CE::Array<>, 
            CE::ScriptEvent<>, CE::ScriptDelegate<>, CE::PropertyBinding<>,
            CE::Vec2, CE::Vec3, CE::Vec4, CE::Vec2i, CE::Vec3i, CE::Vec4i, CE::Rect,
            CE::Quat, CE::Matrix4x4,
			CE::Color, CE::Gradient,
			CE::ObjectMap,
			CE::BinaryBlob,
			CE::SubClassType<>,
			// TypeInfo types
			TypeInfo, EnumType, EnumConstant, StructType, ClassType, FieldType, FunctionType);

        // Enums
        CE_REGISTER_TYPES(
			IO::FileAction,
            EventResult
        );

        // Register Objects
        CE_REGISTER_TYPES(
            Object,
            Bundle,
			Asset,
            DataAsset,
			ResourceManager,
			Resource,
			Settings
        );

    }

}


CE_IMPLEMENT_MODULE(Core, CE::CoreModule)
