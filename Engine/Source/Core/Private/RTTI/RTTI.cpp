
#include "CoreMinimal.h"

namespace CE
{
    class Component;

    namespace Internal
    {
    
        CORE_API TypeId GetArrayTypeId()
        {
            return (TypeId)typeid(CE::Array<u8>).hash_code();
        }
    
        CORE_API TypeId GetObjectStoreTypeId()
        {
            return (TypeId)typeid(CE::ObjectMap).hash_code();
        }

        CORE_API TypeId GetSubClassTypeTypeId()
        {
			return (TypeId)typeid(CE::SubClassType<Object>).hash_code();
        }

        CORE_API TypeId GetPropertyBindingTypeId()
        {
            return typeid(CE::PropertyBinding<>).hash_code();
        }

        CORE_API TypeId GetScriptDelegateTypeId()
        {
            return (TypeId)typeid(CE::ScriptDelegate<void(void)>).hash_code();
        }

        CORE_API TypeId GetScriptEventTypeId()
        {
            return (TypeId)typeid(CE::ScriptEvent<void(void)>).hash_code();
        }

    }

    HashMap<Name, TypeInfo*> TypeInfo::registeredTypesByName{};
    HashMap<TypeId, TypeInfo*> TypeInfo::registeredTypeById{};

    Array<Name> TypeInfo::currentlyLoadingModuleStack{};
    Array<Name> TypeInfo::currentlyUnloadingModuleStack{};

    HashMap<Name, Array<TypeInfo*>> TypeInfo::registeredTypesByModuleName{};

    String Attribute::CleanupAttributeString(const String& inString)
    {
        if (inString.GetLength() == 0)
            return "";

        char* result = new char[inString.GetLength() + 1];
        result[inString.GetLength()] = 0;
        bool isString = false;
        int curParenScope = 0;
        int idx = 0;
        
        for (int i = 0; i < inString.GetLength(); i++)
        {
            if (inString[i] == '"')
            {
                isString = !isString;
                continue;
            }
            
            if (inString[i] == ' ' && !isString)
            {
                continue;
            }

            if (!isString && idx == 0 && inString[i] == '(')
            {
                continue;
            }
            if (!isString && curParenScope == 0 && inString[i] == ')')
            {
                continue;
            }

            if (!isString && inString[i] == '(')
            {
                curParenScope++;
            }
            if (!isString && inString[i] == ')')
            {
                curParenScope--;
            }
            
            result[idx++] = inString[i];
        }

        result[idx++] = 0; // terminating char

        auto str = String(result);
        delete result;
        return str;
    }

    bool Attribute::IsString() const
    {
        return isString;
    }

    bool Attribute::IsMap() const
    {
        return isMap;
    }

    void Attribute::Parse(String attributes, Attribute& outAttrib, bool cleanup)
    {
        outAttrib = Attribute();
        outAttrib.isMap = true;
        outAttrib.tableValue = {};

        int curScope = 0;
        bool isString = false;
        if (cleanup)
            attributes = CleanupAttributeString(attributes);

        int startIdx = 0;

        for (int i = 0; i < attributes.GetLength(); i++)
        {
            if (attributes[i] == '(' || attributes[i] == '[' || attributes[i] == '{')
            {
                curScope++;
                continue;
            }
            else if (attributes[i] == ')' || attributes[i] == ']' || attributes[i] == '}')
            {
                curScope--;
                if (i < attributes.GetLength() - 1)
                    continue;
            }

            bool isLast = i == attributes.GetLength() - 1;

            if ((attributes[i] == ',' && curScope <= 0) || isLast)
            {
                auto attribString = attributes.GetSubstringView(startIdx, i - startIdx + (isLast ? 1 : 0));

                String attribKey = "";
                String attribValue = "";
                bool keyFinished = false;
                int innerScope = 0;

                for (int j = 0; j < attribString.GetSize(); j++)
                {
                    if (attribString[j] == '"')
                    {
                        continue;
                    }
                    if (!keyFinished && attribString[j] == '=')
                    {
                        keyFinished = true;
                        continue;
                    }

                    if (!keyFinished)
                    {
                        attribKey.Append(attribString[j]);
                    }
                    else
                    {
                        attribValue.Append(attribString[j]);
                    }
                }
                
                if (attribValue.StartsWith("(") && attribValue.EndsWith(")"))
                {
                    Attribute subAttrib;
                    Parse(attribValue.GetSubstringView(1, attribValue.GetLength() - 2), subAttrib, false);
                    outAttrib.tableValue.Add({ attribKey, subAttrib });
                }
                else
                {
                    outAttrib.tableValue.Add({ attribKey, Attribute(attribValue) });
                }

                startIdx = i + 1;
            }
        }
    }

    TypeInfo::TypeInfo(CE::Name name, CE::String attributes)
        : name(name), originalAttributes(attributes)
    {
        int curScope = 0;
        bool isString = false;

        Attribute::Parse(attributes, this->attributes);
    }

    String TypeInfo::GetDisplayName()
    {
        if (!displayName.IsEmpty())
            return displayName;

		if (HasAttribute("DisplayName") && GetAttribute("DisplayName").IsString())
		{
			displayName = GetAttribute("DisplayName").GetStringValue();
			if (!displayName.IsEmpty())
				return displayName;
		}

        auto tempStr = name.GetLastComponent();
        displayName = "";

        for (int i = 0; i < tempStr.GetLength(); i++)
        {
            if (i == 0)
            {
                displayName.Append((char)std::toupper(tempStr[i]));
                continue;
            }
            if (std::isupper(tempStr[i]) && !std::isupper(tempStr[i - 1]))
            {
                displayName.Append(' ');
                displayName.Append(tempStr[i]);
            }
            else
            {
                displayName.Append(tempStr[i]);
            }
        }

        if (displayName.StartsWith("m_"))
            displayName.Remove(0, 2);

        return displayName;
    }

    const Attribute& TypeInfo::GetAttributes()
    {
        return attributes;
    }

    Attribute& TypeInfo::GetAttribute(const Name& key)
    {
        static Attribute empty{};
        
        return attributes.HasKey(key) ? attributes.GetKeyValue(key) : empty;
    }

    bool TypeInfo::HasAttribute(const Name& key)
    {
        return attributes.HasKey(key);
    }

	u32 TypeInfo::GetFieldSize()
	{
		if (IsClass()) // Classes are always pointers
		{
			return sizeof(Object*);
		}

		return GetSize();
	}

    bool TypeInfo::IsVectorType() const
    {
        auto typeId = this->GetTypeId();
        return typeId == TYPEID(Vec2) || typeId == TYPEID(Vec2i) ||
            typeId == TYPEID(Vec3) || typeId == TYPEID(Vec3i) ||
            typeId == TYPEID(Vec4) || typeId == TYPEID(Vec4i);
    }

	Name TypeInfo::GenerateInstanceName(Uuid uuid) const
    {
        String typeName = GetName().GetString();
        return String::Format("{0}_{1:x}", typeName, uuid);
    }

    Name TypeInfo::GetOwnerModuleName() const
    {
        return registeredModuleName;
    }

    void TypeInfo::FireTypeRegistrationEventsForCurrentModule()
    {
		if (currentlyLoadingModuleStack.IsEmpty())
			return;

		const auto& typesInThisModule = registeredTypesByModuleName[currentlyLoadingModuleStack.Top()];

		for (auto type : typesInThisModule)
		{
			if (type->IsClass())
			{
				CoreObjectDelegates::onClassRegistered.Broadcast((ClassType*)type);
			}
			else if (type->IsStruct())
			{
				CoreObjectDelegates::onStructRegistered.Broadcast((StructType*)type);
			}
			else if (type->IsEnum())
			{
				CoreObjectDelegates::onEnumRegistered.Broadcast((EnumType*)type);
			}
		}
    }

    void TypeInfo::RegisterType(TypeInfo* type)
    {
        if (type == nullptr || registeredTypeById.KeyExists(type->GetTypeId()))
            return;
        
        registeredTypesByName.Add({ type->GetTypeName(), type});
        registeredTypeById.Add({ type->GetTypeId(), type });

        if (currentlyLoadingModuleStack.NotEmpty())
        {
            if (!registeredTypesByModuleName.KeyExists(currentlyLoadingModuleStack.Top()))
                registeredTypesByModuleName.Add({ currentlyLoadingModuleStack.Top(), {} });

            registeredTypesByModuleName[currentlyLoadingModuleStack.Top()].Add(type);
            type->registeredModuleName = currentlyLoadingModuleStack.Top();
        }

        if (type->IsStruct())
        {
            auto structType = (StructType*)type;
            StructType::RegisterStructType(structType);
        }
        else if (type->IsClass())
        {
            auto classType = (ClassType*)type;
            ClassType::RegisterClassType(classType);
        }
        else if (type->IsEnum())
        {
            auto enumType = (EnumType*)type;
            EnumType::RegisterEnumType(enumType);
        }
    }

    void TypeInfo::DeregisterType(TypeInfo* type)
    {
        if (type == nullptr)
            return;

        if ((currentlyUnloadingModuleStack.IsEmpty() && type->registeredModuleName.IsValid()) ||
            (currentlyUnloadingModuleStack.NotEmpty() && currentlyUnloadingModuleStack.Top() == type->registeredModuleName))
        {
            return;
        }

        registeredTypesByName.Remove(type->GetTypeName());
        registeredTypeById.Remove(type->GetTypeId());

        if (type->registeredModuleName.IsValid() &&
            registeredTypesByModuleName.KeyExists(type->registeredModuleName))
        {
            registeredTypesByModuleName[type->registeredModuleName].Remove(type);
        }

        if (type->IsStruct())
        {
            auto structType = (StructType*)type;
            StructType::DeregisterStructType(structType);
        }
        else if (type->IsClass())
        {
            auto classType = (ClassType*)type;
            ClassType::DeregisterClassType(classType);
        }
        else if (type->IsEnum())
        {
            auto enumType = (EnumType*)type;
            EnumType::DeregisterEnumType(enumType);
        }
    }

    void TypeInfo::DeregisterTypesForModule(ModuleInfo* moduleInfo)
    {
        if (moduleInfo == nullptr || !moduleInfo->moduleName.IsValid())
            return;
        if (!registeredTypesByModuleName.KeyExists(moduleInfo->moduleName))
            return;

        auto& types = registeredTypesByModuleName[moduleInfo->moduleName];

        for (TypeInfo* typeInfo : types)
        {
            if (typeInfo == nullptr)
                continue;

            registeredTypesByName.Remove(typeInfo->GetTypeName());
            registeredTypeById.Remove(typeInfo->GetTypeId());

            if (typeInfo->IsStruct())
            {
                auto structType = (StructType*)typeInfo;
                StructType::DeregisterStructType(structType);
            }
            else if (typeInfo->IsClass())
            {
                auto classType = (ClassType*)typeInfo;
                ClassType::DeregisterClassType(classType);
            }
            else if (typeInfo->IsEnum())
            {
                auto enumType = (EnumType*)typeInfo;
                EnumType::DeregisterEnumType(enumType);
            }
        }

        types.Clear();
    }

    CORE_API TypeInfo* GetTypeInfo(Name name)
    {
        if (TypeInfo::registeredTypesByName.KeyExists(name))
        {
            return TypeInfo::registeredTypesByName[name];
        }
        return nullptr;
    }

    CORE_API TypeInfo* GetTypeInfo(TypeId typeId)
    {
        if (TypeInfo::registeredTypeById.KeyExists(typeId))
        {
            return TypeInfo::registeredTypeById[typeId];
        }
        return nullptr;
    }

}


CE_RTTI_TYPEINFO_IMPL(CE, TypeInfo)

// POD Types

CE_RTTI_POD_IMPL(, b8)
CE_RTTI_POD_IMPL(, c8)
CE_RTTI_POD_IMPL(, c16)

CE_RTTI_POD_IMPL(, s8)
CE_RTTI_POD_IMPL(, s16)
CE_RTTI_POD_IMPL(, s32)
CE_RTTI_POD_IMPL(, s64)

CE_RTTI_POD_IMPL(, u8)
CE_RTTI_POD_IMPL(, u16)
CE_RTTI_POD_IMPL(, u32)
CE_RTTI_POD_IMPL(, u64)
CE_RTTI_POD_IMPL(CE, Uuid)

CE_RTTI_POD_IMPL(, f32)
CE_RTTI_POD_IMPL(, f64)

CE_RTTI_POD_IMPL(CE, String)
CE_RTTI_POD_IMPL(CE, Name)
CE_RTTI_POD_IMPL(CE::IO, Path)

CE_RTTI_POD_TEMPLATE_IMPL(CE, Array, u8)


// Math RTTI

CE_RTTI_POD_IMPL(CE, Vec2i)
CE_RTTI_POD_IMPL(CE, Vec3i)
CE_RTTI_POD_IMPL(CE, Vec4i)

CE_RTTI_POD_IMPL(CE, Vec2)
CE_RTTI_POD_IMPL(CE, Vec3)
CE_RTTI_POD_IMPL(CE, Vec4)
CE_RTTI_POD_IMPL(CE, Rect)

CE_RTTI_POD_IMPL(CE, Quat)

CE_RTTI_POD_IMPL(CE, Matrix4x4)

CE_RTTI_POD_IMPL(CE, Color)
CE_RTTI_POD_IMPL(CE, Gradient)
