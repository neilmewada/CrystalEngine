#include "EditorCore.h"

namespace CE::Editor
{
    static WeakRef<AssetDefinitionRegistry> instance;

    AssetDefinitionRegistry::AssetDefinitionRegistry()
    {
        if (!IsDefaultInstance())
        {
            instance = this;

            classRegHandle = CoreObjectDelegates::onClassRegistered.AddDelegateInstance(MemberDelegate(&Self::OnClassRegistered, this));
			classDeregHandle = CoreObjectDelegates::onClassDeregistered.AddDelegateInstance(MemberDelegate(&Self::OnClassDeregistered, this));

			auto derivedClasses = AssetDefinition::StaticType()->GetDerivedClasses();
			
			for (auto classType : derivedClasses)
			{
				OnClassRegistered(classType);
			}
        }
    }

    AssetDefinitionRegistry::~AssetDefinitionRegistry()
    {
        if (classRegHandle != 0)
        {
            CoreObjectDelegates::onClassRegistered.RemoveDelegateInstance(classRegHandle);
            classRegHandle = 0;
        }

		if (classDeregHandle != 0)
		{
			CoreObjectDelegates::onClassDeregistered.RemoveDelegateInstance(classDeregHandle);
			classDeregHandle = 0;
		}
    }

    Ref<AssetDefinitionRegistry> AssetDefinitionRegistry::Get()
	{
        return instance.Lock();
    }

    AssetDefinition* AssetDefinitionRegistry::FindAssetDefinition(const String& sourceExtension)
	{
		for (auto assetDef : assetDefinitions)
		{
			if (assetDef->GetSourceExtensions().Exists(sourceExtension))
			{
				return assetDef;
			}
		}

		return nullptr;
	}

	AssetDefinition* AssetDefinitionRegistry::FindAssetDefinition(const SubClass<Asset>& assetClass)
	{
        if (assetClass == nullptr)
            return nullptr;

        for (auto assetDef : assetDefinitions)
        {
            if (assetClass->IsSubclassOf(assetDef->GetAssetClass()))
            {
                return assetDef;
            }
        }

        return nullptr;
	}

    void AssetDefinitionRegistry::OnBeginDestroy()
    {
        Super::OnBeginDestroy();

        if (this == instance.Get())
        {
            instance = nullptr;
		}
    }

    void AssetDefinitionRegistry::OnClassRegistered(ClassType* classType)
    {
        if (!classType)
            return;
        
        if (classType != AssetDefinition::StaticType() &&
            classType->IsSubclassOf<AssetDefinition>() &&
            classType->CanBeInstantiated())
        {
			assetDefinitions.Add((AssetDefinition*)classType->GetDefaultInstance());
        }
    }

    void AssetDefinitionRegistry::OnClassDeregistered(ClassType* classType)
    {
        if (!classType)
            return;
        
        if (classType != AssetDefinition::StaticType() &&
            classType->IsSubclassOf<AssetDefinition>() &&
			classType->CanBeInstantiated())
        {
			assetDefinitions.Remove((AssetDefinition*)classType->GetDefaultInstance());
        }
    }

} // namespace CE::Editor

