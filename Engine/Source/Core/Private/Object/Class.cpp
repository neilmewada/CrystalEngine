
#include "CoreMinimal.h"


namespace CE
{

    CE::HashMap<TypeId, StructType*> StructType::registeredStructs{};
    CE::HashMap<Name, StructType*> StructType::registeredStructsByName{};

	//StructType::StructType(const StructType& copy) : TypeInfo(copy.name, copy.originalAttributes)
	//{
	//	localFields = copy.localFields;
	//	localFunctions = copy.localFunctions;

	//	cachedFields = copy.cachedFields;
	//}

	StructType::~StructType()
	{
        LockGuard lock{ cachedFieldsMutex };

        cachedFields.Clear();
        cachedFieldsMap.Clear();
	}

	bool StructType::IsAssignableTo(TypeId typeId)
    {
        if (typeId == this->GetTypeId())
            return true;

        for (auto superTypeId : superTypeIds)
        {
            if (superTypeId == typeId)
            {
                return true;
            }

            auto superType = GetTypeInfo(superTypeId);

            if (superType != nullptr && superType->IsStruct())
            {
                auto structType = (StructType*)superType;
                if (structType->IsAssignableTo(typeId))
                    return true;
            }
            else if (superType != nullptr && superType->IsClass())
            {
                auto classType = (ClassType*)superType;
                if (classType->IsAssignableTo(typeId))
                    return true;
            }
        }

        return false;
    }

    const Attribute& StructType::GetAttributes()
    {
        if (!attributesCached)
        {
            CacheAllAttributes();
        }

        return cachedAttributes;
    }

    bool StructType::HasAttribute(const Name& key)
    {
        if (!attributesCached)
        {
            CacheAllAttributes();
        }

        return cachedAttributes.IsMap() && cachedAttributes.HasKey(key);
    }

    Attribute& StructType::GetAttribute(const CE::Name& key)
    {
        if (!attributesCached)
        {
            CacheAllAttributes();
        }

        if (HasAttribute(key))
            return cachedAttributes.GetKeyValue(Name(key));

	    static Attribute attribute{};

        return attribute;
    }

    Array<FunctionType*> StructType::GetAllFunctions()
    {
		if (!functionsCached)
		{
			CacheAllFunctions();
		}

		Array<FunctionType*> result{};
		for (auto& function : cachedFunctions)
		{
			result.Add(&function);
		}
		return result;
    }

	Array<FunctionType*> StructType::GetAllSignals()
	{
		if (!functionsCached)
		{
			CacheAllFunctions();
		}

		Array<FunctionType*> result{};
		for (auto& function : cachedFunctions)
		{
			if (function.IsSignalFunction())
				result.Add(&function);
		}
		return result;
	}

	bool StructType::HasEventFields()
	{
        if (!fieldsCached)
        {
            CacheAllFields();
        }

        return hasEventFields;
	}

    Ptr<FieldType> StructType::GetFirstField()
    {
        CacheAllFields();

        if (cachedFields.GetSize() == 0)
            return nullptr;

        return cachedFields[0].Get();
    }

    Array<FieldType*> StructType::FetchObjectFields()
    {
		Array<FieldType*> result{};

		auto field = GetFirstField();

		while (field != nullptr)
		{
			if (field->IsObjectField())
			{
				result.Add(field);
			}
			field = field->GetNext();
		}

        return result;
    }

    u32 StructType::GetFieldCount()
    {
        CacheAllFields();

        return cachedFields.GetSize();
    }

    Ptr<FieldType> StructType::GetFieldAt(u32 index)
    {
        CacheAllFields();

        return index < GetFieldCount() ? cachedFields[index].Get() : nullptr;
    }

    Ptr<FieldType> StructType::FindField(const Name& name)
    {
	    ZoneScoped;

        CacheAllFields();

        LockGuard lock{ cachedFieldsMutex };
        
        if (cachedFieldsMap.KeyExists(name) && cachedFieldsMap[name] != nullptr)
		{
            return cachedFieldsMap[name];
		}

        return nullptr;
    }

    bool StructType::FindFieldInstanceRelative(const Name& relativeInstancePath, const Ref<Object>& targetObject,
	    Ptr<FieldType>& outField, void*& outInstance)
    {
	    ZoneScoped;

        if (targetObject.IsNull())
        {
            return false;
        }

        StructType* outFieldOwner = nullptr;
        Ref<Object> outObject;
        return FindFieldInstanceRelative(relativeInstancePath, targetObject, targetObject.Get(),
            outFieldOwner, outField, outObject, outInstance);
    }

    bool StructType::FindFieldInstanceRelative(const Name& relativeInstancePath, const Ref<Object>& targetObject,
                                               Ptr<FieldType>& outField, Ref<Object>& outObject, void*& outInstance)
    {
	    ZoneScoped;

        if (targetObject.IsNull())
        {
            return false;
        }

	    StructType* outFieldOwner = nullptr;

	    return FindFieldInstanceRelative(relativeInstancePath, targetObject, targetObject.Get(),
	        outFieldOwner, outField, outObject, outInstance);
    }

    bool StructType::FindFieldInstanceRelative(const Name& relativePath, 
                                               const Ref<Object>& targetObject,
                                               void* targetInstance,
                                               StructType*& outFieldOwner,
                                               Ptr<FieldType>& outField,
                                               Ref<Object>& outObject,
                                               void*& outInstance)
    {
	    ZoneScoped;

        CacheAllFields();

        Array<String> split = relativePath.GetString().Split('.');

        if (targetInstance == nullptr)
        {
            targetInstance = targetObject.Get();
        }

        StructType* curType = this;
        Ptr<FieldType> curField = nullptr;
        void* curInstance = targetInstance;
        Ref<Object> curObject = targetObject;

        for (int i = 0; i < split.GetSize(); i++)
        {
            const bool isLast = (i == (int)split.GetSize() - 1);
            String fieldName = split[i];
            int arrayIndex = -1;

            if (curType == nullptr)
            {
	            return false;
            }

	        if (fieldName.Contains('[')) // It's an array
	        {
                Array<String> indexSplit;
                fieldName.Split({ "[", "]" }, indexSplit);

                fieldName = indexSplit[0];
                String::TryParse(indexSplit[1], arrayIndex);
	        }

            curField = curType->FindField(fieldName);

            if (curField == nullptr || curInstance == nullptr)
            {
	            return false;
            }

            if (curField->IsArrayField())
            {
                if (arrayIndex < 0)
                {
                    if (isLast)
                    {
	                    break;
                    }

	                return false;
                }

                Array<Ptr<FieldType>> arrayFields = curField->GetArrayFieldListPtr(curInstance);

                if (arrayIndex >= arrayFields.GetSize() || arrayFields.IsEmpty())
                {
	                return false;
                }

                const Array<u8>& rawArray = curField->GetFieldValue<Array<u8>>(curInstance);
                curInstance = (void*)rawArray.GetData();
                curField = arrayFields[arrayIndex];

                if (isLast)
                {
	                break;
                }

                if (curField->IsStructField())
                {
                    curInstance = curField->GetFieldInstance(curInstance);
                    curType = (StructType*)curField->GetDeclarationType();
                }
                else if (curField->IsObjectField())
                {
                    Ref<Object> ref = nullptr;

                    if (curField->IsStrongRefCounted())
                    {
                        ref = curField->GetFieldValue<Ref<Object>>(curInstance);
                    }
                    else if (curField->IsWeakRefCounted())
                    {
                        ref = curField->GetFieldValue<WeakRef<Object>>(curInstance).Lock();
                    }
                    else
                    {
                        ref = curField->GetFieldValue<Object*>(curInstance);
                    }

                    if (ref.IsValid())
                    {
                        curObject = ref;
                        curInstance = ref.Get();
                        curType = ref->GetClass();
                    }
                }
                else // POD
                {
                    if (!isLast)
                    {
                        return false;
                    }
                }
            }
            else if (curField->IsStructField())
            {
                curInstance = curField->GetFieldInstance(curInstance);
                curType = (StructType*)curField->GetDeclarationType();
            }
            else if (curField->IsObjectField())
            {
                Ref<Object> ref = nullptr;

	            if (curField->IsStrongRefCounted())
	            {
                    ref = curField->GetFieldValue<Ref<Object>>(curInstance);
	            }
                else if (curField->IsWeakRefCounted())
                {
                    ref = curField->GetFieldValue<WeakRef<Object>>(curInstance).Lock();
                }
                else
                {
                    ref = curField->GetFieldValue<Object*>(curInstance);
                }

                if (isLast)
                {
                    break;
                }

                if (ref.IsValid())
                {
                    curObject = ref;
                    curInstance = ref.Get();
                    curType = ref->GetClass();
                }
            }
            else // POD
            {
	            if (!isLast)
	            {
		            return false;
	            }
            }
        }

        outField = curField;
        outFieldOwner = curType;
        outInstance = curInstance;
        outObject = curObject;

        return true;
    }

    bool StructType::HasFunctions()
    {
        if (!functionsCached)
        {
            CacheAllFunctions();
        }

        return cachedFunctions.NotEmpty();
    }

    FunctionType* StructType::FindFunctionWithName(const Name& name)
    {
        if (!functionsCached)
            CacheAllFunctions();

        if (cachedFunctionsMap.KeyExists(name) && cachedFunctionsMap[name].GetSize() > 0)
            return cachedFunctionsMap[name][0];

        return nullptr;
    }

    CE::Array<FunctionType*> StructType::FindAllFunctions(const Name& name)
    {
        if (!functionsCached)
            CacheAllFunctions();
        return cachedFunctionsMap[name];
    }

	void StructType::CacheAllAttributes()
    {
		//LockGuard<RecursiveMutex> lock{ rttiMutex };
        if (attributesCached)
            return;

        attributesCached = true;
        LockGuard<SharedMutex> lock{ cachedAttributesMutex };

        cachedAttributes.isMap = true;
        cachedAttributes.tableValue = {};

        for (int i = 0; i < superTypeIds.GetSize(); i++)
        {
            auto superTypeId = superTypeIds[i];
            auto superType = GetTypeInfo(superTypeId);
            if (superType == nullptr)
                continue;

            if (superType->IsStruct())
            {
                auto superStruct = (StructType*)superType;
                superStruct->CacheAllAttributes();
                MergeAttributes(superStruct->cachedAttributes, true);
            }
            else if (superType->IsClass())
            {
                auto superClass = (ClassType*)superType;
                superClass->CacheAllAttributes();
                MergeAttributes(superClass->cachedAttributes, true);
            }
        }

        MergeAttributes(attributes);
    }

    void StructType::MergeAttributes(const Attribute& attribs, bool inherit)
    {
        if (!attribs.IsMap())
            return;

		static const HashSet<Name> disallowInheritance = { "Abstract", "abstract" };

        auto& table = attribs.GetTableValue();

        for (const auto& [name, attr] : table)
        {
			if (inherit && disallowInheritance.Exists(name))
			{
				continue;
			}

			auto& tableValue = cachedAttributes.GetTableValue();

            if (cachedAttributes.HasKey(name))
            {
				tableValue[name] = attr;
            }
            else
            {
				tableValue.Add({ name, attr });
            }
        }
    }

    void StructType::CacheAllFields()
    {
        if (fieldsCached)
            return;

        LockGuard lock{ cachedFieldsMutex };

        fieldsCached = true;

        cachedFields.Clear();

        for (int i = 0; i < superTypeIds.GetSize(); i++)
        {
            auto typeId = superTypeIds[i];
            if (typeId == this->GetTypeId())
                continue;

            const TypeInfo* type = GetTypeInfo(typeId);
            if (type->IsStruct())
            {
                auto structType = (StructType*)type;
                structType->CacheAllFields();

                for (auto& cachedField : structType->cachedFields)
                {
                    cachedFields.Add(FieldType::Clone(cachedField));
                }
            }
            else if (type->IsClass())
            {
                auto classType = (ClassType*)type;
                classType->CacheAllFields();

                for (auto& cachedField : classType->cachedFields)
                {
                    cachedFields.Add(FieldType::Clone(cachedField));
                }
            }
        }

        for (const auto& localField : localFields)
        {
            cachedFields.Add(FieldType::Clone(localField));
        }

        //cachedFields.AddRange(localFields);

        for (int i = 0; i < cachedFields.GetSize(); i++)
        {
            cachedFields[i]->instanceOwner = this;

            if (cachedFields[i]->IsEventField())
            {
                hasEventFields = true;
            }

            if (i == cachedFields.GetSize() - 1)
            {
                cachedFields[i]->next = nullptr;
            }
            else
            {
                cachedFields[i]->next = cachedFields[i + 1];
            }

			cachedFieldsMap[cachedFields[i]->GetName()] = cachedFields[i];
        }
		
    }

    void StructType::CacheAllFunctions()
    {
        if (functionsCached)
            return;

	    LockGuard lock{ cachedFunctionsMutex };

        functionsCached = true;

        cachedFunctions.Clear();
		cachedFunctionsMap.Clear();

        for (int i = 0; i < superTypeIds.GetSize(); i++)
        {
            auto typeId = superTypeIds[i];
            if (typeId == this->GetTypeId())
                continue;

            const TypeInfo* type = GetTypeInfo(typeId);
            if (type->IsStruct())
            {
                auto structType = (StructType*)type;
                structType->CacheAllFunctions();

                for (const auto& func : structType->cachedFunctions)
                {
                    //cachedFunctions.RemoveAll([&](const FunctionType& f) -> bool
                    //    {
                    //        return (func.GetFunctionSignature() == f.GetFunctionSignature()) && func.GetName() == f.GetName();
                    //    });

					cachedFunctions.Add(func);
                }
            }
            else if (type->IsClass())
            {
                auto classType = (ClassType*)type;
                classType->CacheAllFunctions();

                for (const auto& func : classType->cachedFunctions)
                {
					//cachedFunctions.RemoveAll([&](const FunctionType& f) -> bool
					//	{
					//		return (func.GetFunctionSignature() == f.GetFunctionSignature()) && func.GetName() == f.GetName();
					//	});

                    cachedFunctions.Add(func);
                }
            }
        }

		cachedFunctions.AddRange(localFunctions);

		for (int i = 0; i < cachedFunctions.GetSize(); i++)
		{
			cachedFunctions[i].instanceOwner = this;

			if (i == cachedFunctions.GetSize() - 1)
			{
				cachedFunctions[i].next = nullptr;
			}
			else
			{
				cachedFunctions[i].next = &cachedFunctions[i + 1];
			}

			if (cachedFunctionsMap.KeyExists(cachedFunctions[i].GetName()))
				cachedFunctionsMap[cachedFunctions[i].GetName()].Add(&cachedFunctions[i]);
			else
				cachedFunctionsMap.Add({ cachedFunctions[i].GetName(), { &cachedFunctions[i] } });
		}
    }

    void StructType::RegisterStructType(StructType* type)
    {
        if (type == nullptr || registeredStructs.KeyExists(type->GetTypeId()))
            return;

        registeredStructs.Add({type->GetTypeId(), type});
        registeredStructsByName.Add({ type->GetTypeName(), type });

		if (TypeInfo::currentlyLoadingModuleStack.IsEmpty())
			CoreObjectDelegates::onStructRegistered.Broadcast(type);
    }

    void StructType::DeregisterStructType(StructType* type)
    {
        if (type == nullptr || !registeredStructs.KeyExists(type->GetTypeId()))
            return;

        CoreObjectDelegates::onStructDeregistered.Broadcast(type);
        
        type->fieldsCached = false;
        type->attributesCached = false;
        type->functionsCached = false;

        registeredStructs.Remove(type->GetTypeId());
        registeredStructsByName.Remove(type->GetTypeName());
    }

    StructType* StructType::FindStructByName(Name structName)
    {
        if (!structName.IsValid() || !registeredStructsByName.KeyExists(structName))
            return nullptr;

        return registeredStructsByName[structName];
    }

    StructType* StructType::FindStructById(TypeId structTypeId)
    {
        if (structTypeId == 0 || !registeredStructs.KeyExists(structTypeId))
            return nullptr;

        return registeredStructs[structTypeId];
    }

    CE::HashMap<TypeId, ClassType*> ClassType::registeredClasses{};
    CE::HashMap<Name, ClassType*> ClassType::registeredClassesByName{};
    CE::HashMap<TypeId, Array<TypeId>> ClassType::derivedClassesMap{};

	ClassType::ClassType(String name, Internal::IClassTypeImpl* impl, u32 size, String attributes) 
		: StructType(name, nullptr, size, attributes), Impl(impl)
	{
		ConstructInternal();
	}

	void ClassType::ConstructInternal()
	{

	}

	bool ClassType::IsSubclassOf(TypeId classTypeId)
    {
        if (this->GetTypeId() == classTypeId)
            return true;

        if (!superTypesCached)
        {
            CacheSuperTypes();
        }

        for (int i = 0; i < superClasses.GetSize(); i++)
        {
            if (superClasses[i]->IsSubclassOf(classTypeId))
                return true;
        }

        return false;
    }

    bool ClassType::IsObject()
    {
        return IsSubclassOf(TYPEID(Object));
    }

    void ClassType::CacheSuperTypes()
    {
        if (superTypesCached)
            return;

        bool isModuleLoaded = ModuleManager::Get().IsModuleLoaded(GetClassModule());
        if (!isModuleLoaded)
        {
            CE_LOG(Warn, All, "ClassType accessed without loading the module the class belongs to!");
        }

		superTypesCached = true;

        for (int i = 0; i < superTypeIds.GetSize(); i++)
        {
            auto type = GetTypeInfo(superTypeIds[i]);
            if (type == nullptr)
            {
                superTypesCached = false;
            }

            if (type == nullptr || !type->IsClass())
            {
	            continue;
            }

			superClasses.Add((ClassType*)type);
        }

		AddDerivedClassToMap(this, this);
		
    }

	void ClassType::ClearDefaultInstancesForModule(const Name& moduleName)
	{
		const auto& typesInThisModule = TypeInfo::registeredTypesByModuleName[moduleName];

		for (auto type : typesInThisModule)
		{
			if (type->IsClass())
			{
                ClassType* clazz = (ClassType*)type;

                if (clazz->defaultInstance != nullptr)
				{
					clazz->defaultInstance->BeginDestroy();
                	clazz->defaultInstance = nullptr;
				}
			}
		}
	}

	void ClassType::AddDerivedClassToMap(ClassType* derivedClass, ClassType* parentSearchPath)
    {
        if (derivedClass == nullptr || parentSearchPath == nullptr)
            return;

        for (int i = 0; i < parentSearchPath->GetSuperTypesCount(); i++)
        {
            auto parentTypeId = parentSearchPath->GetSuperType(i);
            auto parentType = GetTypeInfo(parentTypeId);
            if (parentType == nullptr || !parentType->IsClass())
                continue;

            if (!derivedClassesMap.KeyExists(parentTypeId))
                derivedClassesMap.Add({ parentTypeId, {} });
            if (!derivedClassesMap[parentTypeId].Exists(derivedClass->GetTypeId()))
                derivedClassesMap[parentTypeId].Add(derivedClass->GetTypeId());

            AddDerivedClassToMap(derivedClass, (ClassType*)parentType);
        }
    }

	const Object* ClassType::GetDefaultInstance()
	{
		// TODO: In runtime builds, CDI's should be serialized directly instead of loading config files

		if (defaultInstance == nullptr && CanBeInstantiated())
		{
			auto nameString = GetName().GetLastComponent();

			auto moduleName = GetOwnerModuleName().GetString();
			Bundle* transientBundle = ModuleManager::Get().GetLoadedModuleTransientBundle(moduleName);
			if (transientBundle == nullptr)
				transientBundle = GetGlobalTransient();
			
			defaultInstance = CreateObject<Object>(transientBundle, "CDI_" + nameString, OF_ClassDefaultInstance, this, nullptr);
		}
		
		return defaultInstance.Get();
	}

    void ClassType::RegisterClassType(ClassType* type)
    {
        if (type == nullptr || registeredClasses.KeyExists(type->GetTypeId()))
            return;
		
        registeredClasses.Add({type->GetTypeId(), type});
        registeredClassesByName.Add({ type->GetTypeName(), type });

        if (!derivedClassesMap.KeyExists(type->GetTypeId()))
        {
            derivedClassesMap.Add({type->GetTypeId(), {}});
        }

		// Only fire registration event for types that are registered outside any module
		// Types that are registered within a module will fire only after all types within that module are fully loaded
		if (TypeInfo::currentlyLoadingModuleStack.IsEmpty())
		{
			CoreObjectDelegates::onClassRegistered.Broadcast(type);
		}
    }

    void ClassType::DeregisterClassType(ClassType* type)
    {
        if (type == nullptr || !registeredClasses.KeyExists(type->GetTypeId()))
            return;

        CoreObjectDelegates::onClassDeregistered.Broadcast(type);

        type->defaultInstance = nullptr;
        type->fieldsCached = false;
        type->attributesCached = false;
        type->functionsCached = false;

        registeredClasses.Remove(type->GetTypeId());
        registeredClassesByName.Remove(type->GetTypeName());

        if (TypeInfo::currentlyLoadingModuleStack.IsEmpty() && type->defaultInstance.Get())
        {
            type->defaultInstance->BeginDestroy();
            type->defaultInstance = nullptr;
        }
    }

	void ClassType::CreateDefaultInstancesForCurrentModule()
	{
		if (TypeInfo::currentlyLoadingModuleStack.IsEmpty())
			return;

		const auto& typesInThisModule = TypeInfo::registeredTypesByModuleName[TypeInfo::currentlyLoadingModuleStack.Top()];

		for (auto type : typesInThisModule)
		{
			if (type->IsClass())
			{
				((ClassType*)type)->GetDefaultInstance();
			}
		}
	}

	void ClassType::CacheTypesForCurrentModule()
	{
		if (TypeInfo::currentlyLoadingModuleStack.IsEmpty())
			return;

		const auto& typesInThisModule = TypeInfo::registeredTypesByModuleName[TypeInfo::currentlyLoadingModuleStack.Top()];

		for (auto type : typesInThisModule)
		{
			if (type->IsClass() || type->IsStruct())
			{
				((StructType*)type)->CacheAllFields();
				((StructType*)type)->CacheAllAttributes();
				((StructType*)type)->CacheAllFunctions();
			}
			if (type->IsClass())
			{
				((ClassType*)type)->CacheSuperTypes();
			}
		}
	}

    ClassType* ClassType::FindClassByName(const Name& className)
    {
        if (!className.IsValid() || !registeredClassesByName.KeyExists(className))
            return nullptr;

        return registeredClassesByName[className];
    }

    ClassType* ClassType::FindClassById(TypeId classTypeId)
    {
        if (classTypeId == 0 || !registeredClasses.KeyExists(classTypeId))
            return nullptr;

        return registeredClasses[classTypeId];
    }

    Array<TypeId> ClassType::GetDerivedClassesTypeId() const
    {
        if (!derivedClassesMap.KeyExists(GetTypeId()))
            return {};

        return derivedClassesMap[GetTypeId()];
    }

    Array<ClassType*> ClassType::GetDerivedClasses() const
    {
        auto typeIdArray = GetDerivedClassesTypeId();

        Array<ClassType*> result{};

        for (int i = 0; i < typeIdArray.GetSize(); i++)
        {
            auto type = GetTypeInfo(typeIdArray[i]);
            if (type == nullptr || !type->IsClass())
                continue;

            result.Add((ClassType*)type);
        }

        return result;
    }

    Array<ClassType*> ClassType::GetDirectDerivedClasses() const
    {
        auto typeIdArray = GetDerivedClassesTypeId();

        Array<ClassType*> result{};

        for (int i = 0; i < typeIdArray.GetSize(); i++)
        {
            auto type = GetTypeInfo(typeIdArray[i]);
            if (type == nullptr || !type->IsClass())
                continue;
            ClassType* clazz = (ClassType*)type;
            if (clazz->GetSuperClassCount() == 0)
                continue;

        	if (clazz->GetSuperType(0) == GetTypeId())
            {
	            result.Add((ClassType*)type);
            }
        }

        return result;
    }
} // namespace CE

CE_RTTI_TYPEINFO_IMPL(CE, StructType)
CE_RTTI_TYPEINFO_IMPL(CE, ClassType)

