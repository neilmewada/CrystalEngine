#pragma once

namespace CE
{

    class CORE_API Settings : public Object
    {
        CE_CLASS(Settings, Object)
    public:
        
        Settings() = default;
        
        static Ref<Settings> LoadSettings(ClassType* settingsClass, String settingsName = "");

        static void SaveSettings();

        virtual String GetTitleName();

#if PAL_TRAIT_BUILD_EDITOR
        static void SaveSettings(const IO::Path& customPath);
#endif

		static Array<ClassType*> GetSettingsClassesWithCategory(const String& settingsCategory);

		FORCE_INLINE static const Array<ClassType*>& GetAllSettingsClasses() { return settingsClasses; }
        
	private:

		static void OnClassRegistered(ClassType* classType);
		static void OnClassDeregistered(ClassType* classType);

		static Array<ClassType*> settingsClasses;

        friend class CoreModule;
    };

	template<typename TSettings> requires TIsBaseClassOf<Settings, TSettings>::Value
	Ref<TSettings> GetSettings()
	{
		return Object::CastTo<TSettings>(Settings::LoadSettings(TSettings::Type()));
	}
    
	FORCE_INLINE void SaveSettings()
	{
		Settings::SaveSettings();
	}

#if PAL_TRAIT_BUILD_EDITOR
    FORCE_INLINE void SaveSettings(const IO::Path& outBundlePath)
	{
        Settings::SaveSettings(outBundlePath);
	}
#endif
}


CE_RTTI_CLASS(CORE_API, CE, Settings,
    CE_SUPER(CE::Object),
    CE_ABSTRACT,
    CE_ATTRIBS(),
    CE_FIELD_LIST(
                  
    ),
    CE_FUNCTION_LIST()
)
