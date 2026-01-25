
#include "CoreMinimal.h"

namespace CE
{
	namespace Internal
	{

		CORE_API Object* CreateObjectInternal(const ObjectCreateParams& params)
		{
			ZoneScoped;

			if (params.objectClass == nullptr || !params.objectClass->CanBeInstantiated() || !params.objectClass->IsObject())
				return nullptr;

			const auto& objectName = params.name;
			if (!IsValidObjectName(objectName) && !params.objectClass->IsSubclassOf<Bundle>())
			{
				CE_LOG(Error, All, "Failed to create object. Invalid name passed: {}", objectName);
				return nullptr;
			}
			
			ObjectCreateParams createInfo = ObjectCreateParams();
			createInfo.objectFlags = params.objectFlags | OF_InsideConstructor;
			createInfo.name = params.name;
			createInfo.uuid = params.uuid;
			createInfo.objectClass = params.objectClass;

			GetObjectCreationContext()->GetStorage().Push(&createInfo);

			Object* instance = params.objectClass->CreateInstance(); // The constructor automatically pops ObjectCreateParams from stack
			if (instance == nullptr)
            {
				GetObjectCreationContext()->GetStorage().Pop();
                return nullptr;
            }

			instance->objectFlags &= ~OF_InsideConstructor;

			if (instance->HasAnyObjectFlags(OF_ClassDefaultInstance)) // Class Default Instance
			{
				instance->LoadDefaults();
			}
			else // Load default values from CDI
			{
				instance->LoadFromTemplate(const_cast<Object*>(instance->GetClass()->GetDefaultInstance()));
			}

			if (params.templateObject != nullptr)
			{
				instance->LoadFromTemplate(params.templateObject);
			}
            
			if (params.outer != nullptr)
			{
				params.outer->AttachSubobject(instance);
			}

			instance->OnAfterConstructInternal();

			return instance;
		}
		
	}

	/*
	*	Globals
	*/

    extern WeakRef<Bundle> gSettingsBundle;
	extern ResourceManager* gResourceManager;

	CORE_API Bundle* GetGlobalTransient()
	{
		return ModuleManager::Get().GetLoadedModuleTransientBundle(MODULE_NAME);
	}

	CORE_API Bundle* GetTransient(const String& moduleName)
	{
		return ModuleManager::Get().GetLoadedModuleTransientBundle(moduleName);
	}

	static Ref<Bundle> LoadSettingsBundle()
	{
		LoadBundleArgs loadArgs{
			.loadFully = true,
			.forceReload = true,
			.destroyOutdatedObjects = true
		};
		return Bundle::LoadBundle(GetGlobalTransient(), Name("/Game/Settings"), loadArgs);
	}

	CORE_API Ref<Bundle> GetSettingsBundle()
    {
		if (gSettingsBundle == nullptr)
			gSettingsBundle = LoadSettingsBundle();

		if (gSettingsBundle == nullptr)
			gSettingsBundle = CreateObject<Bundle>(GetGlobalTransient(), TEXT("/Game/Settings"));

        return gSettingsBundle.Lock();
    }

	CORE_API void UnloadSettings()
	{
		if (gSettingsBundle != nullptr)
		{
			gSettingsBundle->BeginDestroy();
			gSettingsBundle = nullptr;
		}
	}

	CORE_API ResourceManager* GetResourceManager()
	{
		if (gResourceManager == nullptr)
			gResourceManager = CreateObject<ResourceManager>(GetTransient(MODULE_NAME), TEXT("ResourceManager"), OF_Transient);
		return gResourceManager;
	}

	CORE_API bool IsValidObjectName(const String& name)
	{
		if (name.IsEmpty())
			return false;

		for (int i = 0; i < name.GetLength(); i++)
		{
			char ch = name[i];
			if (!String::IsAlphabet(ch) && !String::IsNumeric(ch) && ch != '_' && ch != '-')
			{
				return false;
			}
		}
		return true;
	}

	CORE_API String FixObjectName(const String& name)
	{
		return name.Replace({ '\\', '/', ' ', '+', '=', ':', ';', '.', ',', '[', ']', '{', '}', '(', ')',
			'!', '@', '#', '$', '%', '^', '&', '*', '`', '~', '|' },
			'_');
	}

	/* ***************************************
	*	Delegates
	*/

	CoreObjectDelegates::ClassRegistrationDelegate CoreObjectDelegates::onClassRegistered{};
	CoreObjectDelegates::ClassRegistrationDelegate CoreObjectDelegates::onClassDeregistered{};

	CoreObjectDelegates::StructRegistrationDelegate CoreObjectDelegates::onStructRegistered{};
	CoreObjectDelegates::StructRegistrationDelegate CoreObjectDelegates::onStructDeregistered{};

	CoreObjectDelegates::EnumRegistrationDelegate CoreObjectDelegates::onEnumRegistered{};
	CoreObjectDelegates::EnumRegistrationDelegate CoreObjectDelegates::onEnumDeregistered{};

} // namespace CE
