#pragma once

namespace CE::Editor
{
    class AssetDefinition;

    CLASS()
    class EDITORCORE_API AssetDefinitionRegistry : public Object
    {
        CE_CLASS(AssetDefinitionRegistry, Object)
    public:
        
        AssetDefinitionRegistry();
        ~AssetDefinitionRegistry();

		AssetDefinition* FindAssetDefinition(const String& sourceExtension);

        AssetDefinition* FindAssetDefinition(const SubClass<Asset>& assetClass);

        int GetAssetDefinitionsCount() { return assetDefinitions.GetSize(); }

        AssetDefinition* GetAssetDefinition(int index) { return assetDefinitions[index]; }
        
    protected:
        
    private:
        
        void OnClassRegistered(ClassType* classType);
        void OnClassDeregistered(ClassType* classType);
        
        DelegateHandle classRegHandle = 0;
        DelegateHandle classDeregHandle = 0;
        
        Array<AssetDefinition*> assetDefinitions{};

    };
    
} // namespace CE::Editor

#include "AssetDefinitionRegistry.rtti.h"
