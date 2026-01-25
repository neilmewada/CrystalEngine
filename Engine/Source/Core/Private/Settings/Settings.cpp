#include "CoreMinimal.h"

namespace CE
{
	Array<ClassType*> Settings::settingsClasses{};

	static Ref<Settings> LoadSettingsFromBundle(ClassType* settingsClass)
	{
		if (!settingsClass->IsSubclassOf<Settings>() || !settingsClass->CanBeInstantiated())
			return nullptr;
		return Object::CastTo<Settings>(GetSettingsBundle()->LoadObject(settingsClass));
	}

    Ref<Settings> Settings::LoadSettings(ClassType* settingsClass, String settingsName)
    {
        if (!settingsClass->IsSubclassOf<Settings>() || !settingsClass->CanBeInstantiated())
            return nullptr;

		Ref<Settings> settings = LoadSettingsFromBundle(settingsClass);

		if (settings == nullptr)
		{
			if (settingsName.IsEmpty() && settingsClass->HasAttribute("Settings"))
			{
				settingsName = settingsClass->GetAttribute("Settings").GetStringValue();
			}
			if (settingsName.IsEmpty())
			{
				settingsName = settingsClass->GetName().GetLastComponent();
			}

			settings = CreateObject<Settings>(GetSettingsBundle().Get(), settingsName, OF_NoFlags, settingsClass);
		}
		
		return settings;
    }

	void Settings::SaveSettings()
	{
		Ref<Bundle> settingsBundle = GetSettingsBundle();
		if (settingsBundle == nullptr)
			return;

		if (!settingsBundle->IsFullyLoaded())
			settingsBundle->LoadFully();

		Bundle::SaveToDisk(settingsBundle, nullptr);
	}

	String Settings::GetTitleName()
	{
		return GetClass()->GetDisplayName();
	}

#if PAL_TRAIT_BUILD_EDITOR
	void Settings::SaveSettings(const IO::Path& customPath)
	{
		Ref<Bundle> settingsBundle = GetSettingsBundle();
		if (settingsBundle == nullptr)
			return;

		if (!settingsBundle->IsFullyLoaded())
			settingsBundle->LoadFully();

		Bundle::SaveToDisk(settingsBundle, nullptr, customPath);
	}
#endif


	Array<ClassType*> Settings::GetSettingsClassesWithCategory(const String& settingsCategory)
    {
		Array<ClassType*> result{};
		
		for (auto classType : settingsClasses)
		{
			if (classType == nullptr)
				continue;
			if (classType->HasAttribute("SettingsCategory") && classType->GetAttribute("SettingsCategory").IsString())
			{
				String category = classType->GetAttribute("SettingsCategory").GetStringValue();
				if (category == settingsCategory)
					result.Add(classType);
			}
		}
        return result;
    }

	void Settings::OnClassRegistered(ClassType* classType)
	{
		if (classType != nullptr && classType->IsSubclassOf<Settings>() && classType != GetStaticClass<Settings>())
		{
			settingsClasses.Add(classType);
		}
	}

	void Settings::OnClassDeregistered(ClassType* classType)
	{
		if (classType != nullptr && classType->IsSubclassOf<Settings>() && classType != GetStaticClass<Settings>())
		{
			settingsClasses.Remove(classType);
		}
	}
}

CE_RTTI_CLASS_IMPL(CORE_API, CE, Settings)

