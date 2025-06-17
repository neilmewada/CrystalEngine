
#include "CoreMinimal.h"

namespace CE
{

	static HashSet<Ref<Object>> gRootObjects{};
	static SharedMutex gRootObjectsMutex{};

	SharedRecursiveMutex ObjectListener::mutex{};
	HashMap<Uuid, Array<IObjectUpdateListener*>> ObjectListener::listeners{};

	void ObjectListener::AddListener(Uuid target, IObjectUpdateListener* listener)
	{
		LockGuard lock{ mutex };

		listeners[target].Add(listener);
	}

	void ObjectListener::RemoveListener(Uuid target, IObjectUpdateListener* listener)
	{
		LockGuard lock{ mutex };

		listeners[target].Remove(listener);
	}

	void ObjectListener::RemoveAllListeners(Uuid target)
	{
		LockGuard lock{ mutex };

		listeners[target].Clear();
	}

	void ObjectListener::Trigger(Uuid object, const Name& fieldName)
	{
		Array<IObjectUpdateListener*> listenerArray;

		{
			LockGuard lock{ mutex };

			if (!listeners.KeyExists(object))
				return;

			listenerArray = listeners[object];
		}

		for (IObjectUpdateListener* listener : listenerArray)
		{
			listener->OnObjectFieldChanged(object, fieldName);
		}
	}

	Object::Object() : name("Object"), uuid(Uuid::Random())
    {
        control = new Internal::RefCountControl();
        control->object = this;
        control->objectState = Internal::RefCountControl::Alive;
        
        ConstructInternal();
    }

	Object::~Object()
	{
		if (control != nullptr && control->weakReferences > 0)
		{
			String::IsAlphabet('a');
		}
        delete control; control = nullptr;
	}

	void Object::UnbindAllEvents()
	{
		Class* clazz = GetClass();

		if (clazz->HasEventFields())
		{
			for (Ptr<FieldType> field = clazz->GetFirstField(); field != nullptr; field = field->GetNext())
			{
				if (!field->IsEventField())
					continue;

				IScriptEvent* scriptEvent = field->GetFieldEventValue(this);
				scriptEvent->UnbindAll();
			}
		}
	}

	void Object::DestroyImmediate()
	{
		if (!IsPendingDestruction())
		{
			objectFlags |= OF_PendingDestroy;

			ObjectListener::RemoveAllListeners(GetUuid());

			OnBeginDestroy();

			if (!IsDefaultInstance() && !IsTransient() && GetClass()->HasAttribute("Prefs"))
			{
				Prefs::Get().SavePrefs(this);
			}

			auto bundle = GetBundle();
			if (bundle != nullptr)
			{
				bundle->OnObjectUnloaded(this); // Mark the object as 'unloaded'
			}

			// Detach this object from outer
			if (Ref<Object> outerObj = outer.Lock())
			{
				outerObj->DetachSubobject(this);
			}
			outer = nullptr;

			// Delete all attached subobjects
			if (attachedObjects.GetObjectCount() > 0)
			{
				for (int i = attachedObjects.GetObjectCount() - 1; i >= 0; i--)
				{
					auto subobject = attachedObjects.GetObjectAt(i);
					subobject->outer = nullptr;
					subobject->BeginDestroy();
				}
			}
			attachedObjects.RemoveAll();
		}

		OnBeforeDestroy();

		delete this;
	}

	void Object::BeginDestroy()
	{
		if (IsPendingDestruction())
			return;

		objectFlags |= OF_PendingDestroy;

		// Prevent this object from being destroyed immediately on outerObj->DetachSubobject(this) call.
		// That can happen if the parent object has the ONLY strong reference to this object, which can
		// destroy this object while we are in this function.
		Ref<Object> strongRef = this;

		ObjectListener::RemoveAllListeners(GetUuid());

		OnBeginDestroy();

		if (!IsDefaultInstance() && !IsTransient() && GetClass()->HasAttribute("Prefs"))
		{
			Prefs::Get().SavePrefs(this);
		}

		auto bundle = GetBundle();
		if (bundle != nullptr)
		{
			bundle->OnObjectUnloaded(this); // Mark the object as 'unloaded'
		}

		// Detach this object from outer
		if (Ref<Object> outerObj = outer.Lock())
		{
			outerObj->DetachSubobject(this);
		}
		outer = nullptr;

		// Delete all attached subobjects
		if (attachedObjects.GetObjectCount() > 0)
		{
			for (int i = attachedObjects.GetObjectCount() - 1; i >= 0; i--)
			{
				auto subobject = attachedObjects.GetObjectAt(i);
				subobject->outer = nullptr;
				subobject->BeginDestroy();
			}
		}
		attachedObjects.RemoveAll();
		
        if (control == nullptr || control->GetNumStrongRefs() == 0)
		{
			OnBeforeDestroy();

			delete this;
		}
	}

	bool Object::IsOfType(ClassType* classType) const
	{
		if (classType == nullptr)
			return false;

		return GetClass()->IsSubclassOf(classType);
	}

    void Object::ConstructInternal()
    {
        auto initParams = GetObjectCreationContext()->GetStorage().Top();
		CE_ASSERT(initParams != nullptr, "An object was constructed without any initializers set! This usually happens when you construct an object using 'new' operator.");
		GetObjectCreationContext()->GetStorage().Pop();
        ConstructInternal(initParams);
    }

    void Object::ConstructInternal(Internal::ObjectCreateParams* parameters)
    {
        CE_ASSERT(parameters != nullptr, "An object was contructed without any initializers set! This usually happens when you construct an object using 'new' operator.");
		CE_ASSERT(parameters->objectClass != nullptr, "Object initializer passed with null objectClass!");

        this->objectFlags = parameters->objectFlags;
        if (parameters->uuid.IsValid())
            this->uuid = parameters->uuid;
        this->name = parameters->name;
    }

    void Object::SetName(const Name& newName)
    {
		if (IsValidObjectName(newName.GetString()))
		{
			this->name = newName;
		}
    }

    void Object::AddToRoot()
    {
		LockGuard lock{ gRootObjectsMutex };

		gRootObjects.Add(this);
    }

    void Object::RemoveFromRoot()
    {
		LockGuard lock{ gRootObjectsMutex };

		gRootObjects.Remove(this);
    }

    void Object::AttachSubobject(Object* subobject)
    {
        if (subobject == nullptr || attachedObjects.ObjectExists(subobject->GetUuid()))
            return;

		Ref<Object> strongRef = subobject;
		
		// Detach the passed subobject from its parent first
		if (subobject->outer != nullptr && subobject->outer != this)
			subobject->outer->DetachSubobject(subobject);

        attachedObjects.AddObject(subobject);
        subobject->outer = this;

		auto bundle = GetBundle();
		if (bundle != nullptr)
		{
			LockGuard<SharedMutex> lock{ bundle->loadedObjectsMutex };
			bundle->loadedObjectsByUuid[subobject->GetUuid()] = subobject;
		}
		
		if (EnumHasFlag(subobject->objectFlags, OF_DefaultSubobject) && 
			EnumHasFlag(objectFlags, OF_InsideConstructor))
		{
			// Do not call OnSubobjectAttached on default subobjects because they are created from constructors.
			subobject->objectFlags |= OF_SubobjectPending;
		}
		else
		{
			OnSubobjectAttached(subobject);
		}
    }

    void Object::DetachSubobject(Object* subobject)
    {
        if (subobject == nullptr || !attachedObjects.ObjectExists(subobject->GetUuid()))
            return;

		Ref<Object> strongRef = subobject;

        subobject->outer = nullptr;
        attachedObjects.RemoveObject(subobject);

		auto bundle = GetBundle();
		if (bundle != nullptr)
		{
			LockGuard<SharedMutex> lock{ bundle->loadedObjectsMutex };
			bundle->loadedObjectsByUuid.Remove(subobject->GetUuid());
		}

		OnSubobjectDetached(subobject);
    }

	bool Object::HasSubobject(Object* subobject)
	{
		if (subobject == nullptr)
			return false;
		if (attachedObjects.ObjectExists(subobject->GetUuid()))
		{
			return true;
		}
		return false;
	}

    bool Object::IsObjectPresentInHierarchy(Object* searchObject)
    {
		if (searchObject == nullptr)
			return false;
		if (searchObject == this)
			return true;

		for (auto subobject : attachedObjects)
		{
			if (subobject->IsObjectPresentInHierarchy(searchObject))
				return true;
		}

        return false;
    }

	bool Object::ParentExistsInChain(Object* parent) const
	{
		WeakRef<Object> outer = GetOuter();

		while (outer != nullptr)
		{
			if (outer == parent)
				return true;
			outer = outer->GetOuter();
		}

		return false;
	}

	bool Object::IsTransient() const
	{
		return HasAnyObjectFlags(OF_Transient);
	}

    Bundle* Object::GetBundle()
    {
		if (GetClass()->IsSubclassOf<Bundle>())
			return Object::CastTo<Bundle>(this);

        auto outerObject = outer;
        if (outerObject == nullptr)
            return nullptr;
        
        while (outerObject != nullptr)
        {
            if (outerObject->GetClass()->IsSubclassOf<Bundle>())
                return Object::CastTo<Bundle>(outerObject.Get());
            outerObject = outerObject->outer;
        }
        
        return nullptr;
    }

    u64 Object::ComputeMemoryFootprint()
    {
		u64 totalSize = GetClass()->GetSize();
        if (control != nullptr)
        {
            totalSize += sizeof(Internal::RefCountControl);
        }

		for (auto field = GetClass()->GetFirstField(); field != nullptr; field = field->GetNext())
		{
			if (field->IsArrayField())
			{
				TypeInfo* elementType = field->GetUnderlyingType();
				u64 arrayLength = field->GetArraySize(this);
				if (arrayLength > 0 && elementType)
				{
					u64 underlyingTypeSize = elementType->GetSize();

					if (elementType->IsObject() || elementType->IsTypeInfo())
					{
						underlyingTypeSize = sizeof(void*);
					}

					totalSize += arrayLength * underlyingTypeSize;
				}
			}
			else if (field->IsStringField())
			{
				const String& value = field->GetFieldValue<String>(this);
				totalSize += value.GetCapacity();
			}
		}

		for (const auto& object : attachedObjects)
		{
			totalSize += object->ComputeMemoryFootprint();
		}

		return totalSize;
    }

    bool Object::IsBundle() const
    {
        return IsOfType<Bundle>();
    }

    Name Object::GetPathInBundle(Bundle* bundle)
    {
        if (this == bundle || bundle == nullptr)
            return "";
        
        String path = name.GetString();
        
        auto outerObject = outer;
        
        while (outerObject != nullptr && outerObject != bundle)
        {
            auto outerPathName = outerObject->GetName();
            if (outerPathName.IsValid())
                path = outerPathName.GetString() + "." + path;
            outerObject = outerObject->outer;
        }
        
        return path;
    }

	Name Object::GetPathInBundle()
	{
        if (IsBundle())
            return "";
        
		String path = name.GetString();
		
		auto outerObject = outer;

		while (outerObject != nullptr && !outerObject->IsBundle())
		{
			auto outerPathName = outerObject->GetName();
			if (outerPathName.IsValid())
				path = outerPathName.GetString() + "." + path;
			outerObject = outerObject->outer;
		}

		return path;
	}

	void Object::FetchObjectReferences(HashMap<Uuid, Object*>& outReferences)
	{
		for (const auto& subobjectRef : attachedObjects)
		{
			Object* subobject = subobjectRef.Get();

			if (subobject == nullptr || uuid.IsNull())
				continue;

			if (!outReferences.KeyExists(subobject->uuid))
				outReferences[subobject->uuid] = subobject;
			subobject->FetchObjectReferences(outReferences);
		}

		for (auto field = GetClass()->GetFirstField(); field != nullptr; field = field->GetNext())
		{
			if (field->IsObjectField())
			{
				Object* value = nullptr;
				if (field->IsStrongRefCounted())
				{
					value = field->GetFieldValue<Ref<Object>>(this).Get();
				}
				else if (field->IsWeakRefCounted())
				{
					value = field->GetFieldValue<WeakRef<Object>>(this).Get();
				}
				else
				{
					value = field->GetFieldValue<Object*>(this);
				}
				
				if (value != nullptr && !outReferences.KeyExists(value->uuid))
				{
					outReferences[value->uuid] = value;
				}
			}
			else if (field->GetDeclarationTypeId() == TYPEID(ObjectMap))
			{
				const auto& objectMap = field->GetFieldValue<ObjectMap>(this);
				for (const auto& objectRef : objectMap)
				{
					Object* object = objectRef.Get();

					if (object != nullptr)
					{
						outReferences[object->uuid] = object;
					}
				}
			}
			else if (field->IsArrayField())
			{
				auto underlyingType = field->GetUnderlyingType();
				if (underlyingType != nullptr && underlyingType->IsObject())
				{
					if (field->IsStrongRefCounted())
					{
						const Array<Ref<Object>>& array = field->GetFieldValue<Array<Ref<Object>>>(this);
						for (const auto& object : array)
						{
							if (object != nullptr)
							{
								outReferences[object->uuid] = object.Get();
							}
						}
					}
					else if (field->IsWeakRefCounted())
					{
						const Array<WeakRef<Object>>& array = field->GetFieldValue<Array<WeakRef<Object>>>(this);
						for (const auto& object : array)
						{
							if (object != nullptr)
							{
								outReferences[object->uuid] = object.Get();
							}
						}
					}
					else
					{
						const Array<Object*>& array = field->GetFieldValue<Array<Object*>>(this);
						for (Object* object : array)
						{
							if (object != nullptr)
							{
								outReferences[object->uuid] = object;
							}
						}
					}
				}
			}
			else if (field->IsStructField())
			{
				auto structType = (StructType*)field->GetDeclarationType();
				if (structType != nullptr)
				{
					FetchObjectReferencesInStructField(outReferences, structType, field->GetFieldInstance(this));
				}
			}
		}
	}

	void Object::FetchSubObjectsRecursive(Array<Object*>& outSubObjects)
	{
		outSubObjects.Add(this);

		for (const auto& subObject : attachedObjects)
		{
			if (subObject)
			{
				subObject->FetchSubObjectsRecursive(outSubObjects);
			}
		}
	}

	void Object::FetchObjectReferencesInStructField(HashMap<Uuid, Object*>& outReferences, StructType* structType, void* structInstance)
	{
		if (structType == nullptr || structInstance == nullptr)
			return;

		for (auto field = structType->GetFirstField(); field != nullptr; field = field->GetNext())
		{
			if (field->IsObjectField())
			{
				Object* value = nullptr;

				if (field->IsStrongRefCounted())
				{
					value = field->GetFieldValue<Ref<Object>>(structInstance).Get();
				}
				else if (field->IsWeakRefCounted())
				{
					value = field->GetFieldValue<WeakRef<Object>>(structInstance).Get();
				}
				else
				{
					value = field->GetFieldValue<Object*>(structInstance);
				}
				
				if (value != nullptr && !outReferences.KeyExists(value->uuid))
				{
					outReferences[value->uuid] = value;
				}
			}
			else if (field->IsArrayField())
			{
				auto underlyingType = field->GetUnderlyingType();
				if (underlyingType != nullptr && underlyingType->IsObject())
				{
					if (field->IsStrongRefCounted())
					{
						const Array<Ref<Object>>& array = field->GetFieldValue<Array<Ref<Object>>>(structInstance);
						for (const auto& object : array)
						{
							if (object != nullptr)
							{
								outReferences[object->uuid] = object.Get();
							}
						}
					}
					else if (field->IsWeakRefCounted())
					{
						const Array<WeakRef<Object>>& array = field->GetFieldValue<Array<WeakRef<Object>>>(structInstance);
						for (const auto& object : array)
						{
							if (object != nullptr)
							{
								outReferences[object->uuid] = object.Get();
							}
						}
					}
					else
					{
						const Array<Object*>& array = field->GetFieldValue<Array<Object*>>(structInstance);
						for (Object* object : array)
						{
							if (object != nullptr)
							{
								outReferences[object->uuid] = object;
							}
						}
					}
				}
			}
			else if (field->IsStructField())
			{
				auto fieldStructType = (StructType*)field->GetDeclarationType();
				if (fieldStructType != nullptr)
				{
					FetchObjectReferencesInStructField(outReferences, fieldStructType, field->GetFieldInstance(structInstance));
				}
			}
		}
	}

	void Object::LoadConfig(ClassType* configClass, String fileName)
    {
        if (configClass == NULL)
            configClass = GetClass();
		if (!GetClass()->IsSubclassOf(configClass)) // `this` class should always be same as or a subclass of `configClass`
			return;
        
        //if (fileName.IsEmpty() && configClass->HasAttribute("Config"))
        //{
        //    fileName = configClass->GetAttribute("Config").GetStringValue();
        //}
        //if (fileName.IsEmpty())
        //    return;
        
		auto configType = ConfigType(fileName, false);
        auto config = gConfigCache->GetConfigFile(configType);
        auto classTypeName = configClass->GetTypeName();
        
        if (config == nullptr)
            return;
        if (!config->SectionExists(classTypeName))
            return;

        ConfigSection& configSection = config->Get(classTypeName);
		Ptr<FieldType> configField = configClass->GetFirstField();
        
        while (configField != nullptr)
        {
            if (!configField->HasAnyFieldFlags(FIELD_Config))
            {
				configField = configField->GetNext();
                continue;
            }

			auto field = GetClass()->FindField(configField->GetName());
			if (field->GetTypeId() != configField->GetTypeId())
			{
				configField = configField->GetNext();
				continue;
			}
            
            auto fieldTypeId = field->GetDeclarationTypeId();
            auto fieldName = field->GetName().GetString();
			auto fieldDeclType = field->GetDeclarationType();

            if (!configSection.KeyExists(fieldName) || !configSection[fieldName].IsValid())
            {
				configField = configField->GetNext();
                continue;
            }
            
            ConfigValue& configValue = configSection[fieldName];
            String stringValue = configSection[fieldName].GetString();
			if (stringValue.StartsWith("\""))
				stringValue = stringValue.GetSubstringView(1);
			if (stringValue.EndsWith("\""))
				stringValue = stringValue.GetSubstringView(0, stringValue.GetLength() - 1);
            
			if (field->IsEnumField())
			{
				EnumType* enumType = (EnumType*)fieldDeclType;
				EnumConstant* enumValue = enumType->FindConstantWithName(stringValue);
				if (enumValue)
				{
					field->SetFieldEnumValue(this, enumValue->GetValue());
				}
			}
            else if (fieldTypeId == TYPEID(String))
            {
                field->SetFieldValue<String>(this, stringValue);
            }
            else if (fieldTypeId == TYPEID(IO::Path))
            {
                field->SetFieldValue<IO::Path>(this, IO::Path(stringValue));
            }
            else if (fieldTypeId == TYPEID(Name))
            {
                field->SetFieldValue<Name>(this, Name(stringValue));
            }
			else if (fieldTypeId == TYPEID(ClassType))
			{
				ClassType* classType = ClassType::FindClass(stringValue);
				if (classType != nullptr)
					field->SetFieldValue<ClassType*>(this, classType);
			}
			else if (fieldTypeId == TYPEID(StructType))
			{
				StructType* structType = StructType::FindStruct(stringValue);
				if (structType != nullptr)
					field->SetFieldValue<StructType*>(this, structType);
			}
			else if (fieldTypeId == TYPEID(EnumType))
			{
				EnumType* enumType = EnumType::FindEnum(stringValue);
				if (enumType != nullptr)
					field->SetFieldValue<EnumType*>(this, enumType);
			}
			else if (fieldTypeId == TYPEID(SubClassType<>))
			{
				ClassType* classType = ClassType::FindClass(stringValue);
				if (classType != nullptr)
					field->SetFieldValue<SubClassType<>>(this, classType);
				else
					field->SetFieldValue<SubClassType<>>(this, Name(stringValue));
			}
            else if (fieldTypeId == TYPEID(Uuid))
            {
				field->SetFieldValue<Uuid>(this, Uuid::FromString(stringValue));
            }
            else if (field->IsIntegerField())
            {
                s64 value = 0;
                if (String::TryParse(stringValue, value))
                {
                    if (fieldTypeId == TYPEID(u8))
                        field->SetFieldValue<u8>(this, (u8)value);
                    else if (fieldTypeId == TYPEID(u16))
                        field->SetFieldValue<u16>(this, (u16)value);
                    else if (fieldTypeId == TYPEID(u32))
                        field->SetFieldValue<u32>(this, (u32)value);
                    else if (fieldTypeId == TYPEID(u64))
                        field->SetFieldValue<u64>(this, (u64)value);
                    else if (fieldTypeId == TYPEID(s8))
                        field->SetFieldValue<s8>(this, (s8)value);
                    else if (fieldTypeId == TYPEID(s16))
                        field->SetFieldValue<s16>(this, (s16)value);
                    else if (fieldTypeId == TYPEID(s32))
                        field->SetFieldValue<s32>(this, (s32)value);
                    else if (fieldTypeId == TYPEID(s64))
                        field->SetFieldValue<s64>(this, (s64)value);
                }
            }
            else if (field->IsDecimalField())
            {
                f32 value = 0;
                if (String::TryParse(stringValue, value))
                {
                    if (fieldTypeId == TYPEID(f32))
                        field->SetFieldValue<f32>(this, value);
                    else if (fieldTypeId == TYPEID(f64))
                        field->SetFieldValue<f64>(this, value);
                }
            }
            else if (fieldTypeId == TYPEID(bool))
            {
                bool value = false;
                if (String::TryParse(stringValue, value))
                {
                    field->SetFieldValue<bool>(this, value);
                }
            }
			else if (fieldDeclType != nullptr && fieldDeclType->IsStruct())
			{
				StructType* structType = (StructType*)fieldDeclType;
				void* structInstance = field->GetFieldInstance(this);

				const String& string = configValue.GetString();

				if (string.StartsWith("(") && string.EndsWith(")"))
				{
					ConfigParseStruct(string.GetSubstringView(1, string.GetLength() - 2), structInstance, structType);
				}
			}
            else if (field->IsArrayType() && configValue.IsValid())
            {
                Array<String>& array = configValue.GetArray();
                if (array.IsEmpty())
                {
                    auto& arrayValue = const_cast<Array<u8>&>(field->GetFieldValue<Array<u8>>(this));
                    arrayValue.Clear();
                }
                else
                {
					Array<u8>& arrayValue = const_cast<Array<u8>&>(field->GetFieldValue<Array<u8>>(this));
                    bool isMap = false;
                    isMap = array[0].StartsWith("(");
                    
                    TypeInfo* underlyingType = field->GetUnderlyingType();
                    field->ResizeArray(this, array.GetSize());
                    int idx = -1;
                    
                    for (const String& value : array)
                    {
                        idx++;
                        
                        if (isMap)
                        {
                            if (underlyingType != nullptr && underlyingType->IsStruct())
                            {
                                StructType* structType = (StructType*)underlyingType;
                                void* structInstance = &arrayValue[0] + idx * structType->GetSize();
                                
                                ConfigParseStruct(value.GetSubstringView(1, value.GetLength() - 2), structInstance, structType);
                            }
                        }
                        else
                        {
							Array<Ptr<FieldType>> arrayFieldList = field->GetArrayFieldListPtr(this);

							void* arrayInstance = arrayValue.Begin();

							for (int i = 0; i < arrayFieldList.GetSize(); i++)
							{
								Ptr<FieldType> arrayField = arrayFieldList[i];
								ConfigParseField(array[i], arrayInstance, arrayField);
							}
                        }
                    }
                }
            }
            
			configField = configField->GetNext();
        }
    }

    void Object::OnFieldChanged(const Name& fieldName)
    {
		if (EnumHasFlag(objectFlags, OF_InsideConstructor))
			return;

		ObjectListener::Trigger(GetUuid(), fieldName);
    }

    void Object::OnFieldEdited(const Name& fieldName)
    {

    }

    Object* Object::CreateDefaultSubobject(ClassType* classType, const String& name, ObjectFlags flags)
	{
		return CreateObject<Object>(this, name, flags | OF_DefaultSubobject, classType);
	}

	Object* Object::GetDefaultSubobject(ClassType* classType, const String& name)
	{
		for (const auto& object : attachedObjects)
		{
			if (object->GetName() == name && object->GetClass() == classType)
				return object.Get();
		}
		return nullptr;
	}

	void Object::LoadFromTemplate(Object* templateObject)
	{
		HashMap<Uuid, Object*> originalToCloneMap{};

		std::function<void(Object*, Object*)> fetchSubobjects = [&](Object* src, Object* dst)
			{
				if (src == nullptr || dst == nullptr)
					return;

                if (src != nullptr && dst != nullptr)
                    originalToCloneMap[src->GetUuid()] = dst;
                
				for (const auto& srcObject : src->attachedObjects)
				{
					if (srcObject == nullptr)
						continue;
					Ref<Object> dstObject = dst->attachedObjects.FindObject(srcObject->GetName(), srcObject->GetClass());
					if (dstObject == nullptr)
						continue;

					fetchSubobjects(srcObject.Get(), dstObject.Get());
				}
			};

		fetchSubobjects(templateObject, this);

		LoadFromTemplateHelper(originalToCloneMap, templateObject);
	}

	void Object::LoadFromTemplateHelper(HashMap<Uuid, Object*>& originalToClonedObjectMap, Object* templateObject)
	{
		if (templateObject == nullptr)
			return;

		auto templateClass = templateObject->GetClass();
		auto thisClass = this->GetClass();

		if (!thisClass->IsSubclassOf(templateClass))
			return;

		for (auto field = templateClass->GetFirstField(); field != nullptr; field = field->GetNext())
		{
			auto destField = thisClass->FindField(field->GetName());
			if (destField == nullptr)
			{
				destField = thisClass->FindField(field->GetName());
			}
			if (destField->GetTypeId() != field->GetTypeId()) // Type mismatch
				continue;
			if (destField->IsInternal()) // Do NOT modify name & uuid fields
				continue;
			if (destField->GetName() == "outer" && destField->GetOwnerType()->GetTypeId() == TYPEID(Object))
				continue;

			// TODO: Do not modify fields of a Default Instance!

			if (field->GetDeclarationTypeId() == TYPEID(ObjectMap))
			{
				const ObjectMap& srcMap = field->GetFieldValue<ObjectMap>(templateObject);
				ObjectMap& dstMap = const_cast<ObjectMap&>(destField->GetFieldValue<ObjectMap>(this));

				for (const auto& srcObjectRef : srcMap)
				{
					Object* srcObject = srcObjectRef.Get();
					if (srcObject == nullptr)
						continue;
					Object* dstObject = dstMap.FindObject(srcObject->GetName(), srcObject->GetClass()).Get();
					if (dstObject == nullptr)
						return;

					dstObject->LoadFromTemplateHelper(originalToClonedObjectMap, srcObject);
				}
			}
			else if (field->IsArrayField() && field->GetUnderlyingType() != nullptr && field->GetUnderlyingType()->IsObject())
			{
				//const Array<Object*>& srcArray = field->GetFieldValue<Array<Object*>>(templateObject);
				//Array<Object*>& dstArray = const_cast<Array<Object*>&>(field->GetFieldValue<Array<Object*>>(this));

				//for (auto srcObject : srcArray)
				{
					//if (srcObject == nullptr)
					//	continue;

					// TODO: LoadFromTemplateHelper from Array
				}
			}
			else if (field->IsArrayField())
			{
				u32 arraySize = field->GetArraySize(templateObject);
				destField->ResizeArray(this, arraySize);

				const Array<u8>& srcArray = field->GetFieldValue<Array<u8>>(templateObject);
				const Array<u8>& destArray = destField->GetFieldValue<Array<u8>>(this);

				if (arraySize > 0)
				{
					Array<Ptr<FieldType>> srcElements = field->GetArrayFieldListPtr(templateObject);
					Array<Ptr<FieldType>> destElements = destField->GetArrayFieldListPtr(this);
					void* srcInstance = (void*)&srcArray[0];
					void* destInstance = (void*)&destArray[0];

					for (int i = 0; i < arraySize; i++)
					{
						LoadFromTemplateFieldHelper(originalToClonedObjectMap, srcElements[i], srcInstance,
							destElements[i], destInstance);
					}
				}
			}
			else if (field->IsObjectField()) // Deep copy object fields
			{
				Object* objectToCopy = nullptr;

				if (field->IsStrongRefCounted())
				{
					objectToCopy = field->GetFieldValue<Ref<Object>>(templateObject).Get();
				}
				else if (field->IsWeakRefCounted())
				{
					objectToCopy = field->GetFieldValue<WeakRef<Object>>(templateObject).Get();
				}
				else
				{
					objectToCopy = field->GetFieldValue<Object*>(templateObject);
				}
				
				if (objectToCopy == nullptr)
				{
					//destField->SetFieldValue<Object*>(this, nullptr);
				}
				else
				{
					Object* valueToSet = nullptr;
					if (originalToClonedObjectMap.KeyExists(objectToCopy->GetUuid()))
					{
						valueToSet = originalToClonedObjectMap[objectToCopy->GetUuid()];
					}
					else if (!originalToClonedObjectMap.KeyExists(objectToCopy->GetUuid()))
					{
						// Shallow copy external object references
						valueToSet = objectToCopy;
					}

					if (destField->IsStrongRefCounted())
					{
						destField->SetFieldValue<Ref<Object>>(this, valueToSet);
					}
					else if (destField->IsWeakRefCounted())
					{
						destField->SetFieldValue<WeakRef<Object>>(this, valueToSet);
					}
					else
					{
						destField->SetFieldValue<Object*>(this, valueToSet);
					}
				}
			}
			else
			{
				LoadFromTemplateFieldHelper(originalToClonedObjectMap, field, templateObject, destField, this);
			}
		}
	}

	void Object::LoadFromTemplateFieldHelper(HashMap<Uuid, Object*>& originalToClonedObjectMap, 
		const Ptr<FieldType>& srcField, void* srcInstance, const Ptr<FieldType>& dstField, void* dstInstance)
	{
		if (dstField == nullptr || dstField->GetDeclarationTypeId() != srcField->GetDeclarationTypeId() || srcInstance == nullptr || dstInstance == nullptr)
			return;
		if (dstField->IsReadOnly() || srcField->GetDeclarationType() == nullptr)
			return;
		
		auto fieldDeclId = dstField->GetDeclarationTypeId();
		auto fieldDeclType = dstField->GetDeclarationType();
		
		static const Array<TypeId> rawDataTypes{
			TYPEID(Vec2), TYPEID(Vec2i),
			TYPEID(Vec3), TYPEID(Vec3i),
			TYPEID(Vec4), TYPEID(Vec4i), TYPEID(Quat), TYPEID(Color),
			TYPEID(Matrix4x4),
			TYPEID(ClassType), TYPEID(StructType), TYPEID(EnumType), TYPEID(SubClassType<>)
		};

		if (fieldDeclType->IsEnum())
		{
			auto value = srcField->GetFieldEnumValue(srcInstance);
			dstField->ForceSetFieldEnumValue(dstInstance, value);
		}
		else if (fieldDeclType->IsPOD())
		{
			if (fieldDeclId == TYPEID(String))
			{
				String value = srcField->GetFieldValue<String>(srcInstance);
				dstField->ForceSetFieldValue(dstInstance, value);
			}
			else if (fieldDeclId == TYPEID(Name))
			{
				Name value = srcField->GetFieldValue<Name>(srcInstance);
				dstField->ForceSetFieldValue(dstInstance, value);
			}
			else if (fieldDeclId == TYPEID(IO::Path))
			{
				IO::Path value = srcField->GetFieldValue<IO::Path>(srcInstance);
				dstField->ForceSetFieldValue(dstInstance, value);
			}
			else if (fieldDeclId == TYPEID(Uuid))
			{
				Uuid value = srcField->GetFieldValue<Uuid>(srcInstance);
				dstField->ForceSetFieldValue(dstInstance, value);
			}
			else if (srcField->IsArrayField())
			{
				u32 arraySize = srcField->GetArraySize(srcInstance);
				dstField->ResizeArray(dstInstance, arraySize);

				const Array<u8>& srcArray = srcField->GetFieldValue<Array<u8>>(srcInstance);
				const Array<u8>& destArray = dstField->GetFieldValue<Array<u8>>(dstInstance);

				if (arraySize > 0)
				{
					Array<Ptr<FieldType>> srcElements = srcField->GetArrayFieldListPtr(srcInstance);
					Array<Ptr<FieldType>> destElements = dstField->GetArrayFieldListPtr(dstInstance);
					void* sourceInstance = (void*)&srcArray[0];
					void* destinationInstance = (void*)&destArray[0];

					for (int i = 0; i < arraySize; i++)
					{
						LoadFromTemplateFieldHelper(originalToClonedObjectMap, srcElements[i], sourceInstance,
							destElements[i], destinationInstance);
					}
				}
			}
			else if (srcField->IsDecimalField() || srcField->IsIntegerField() || fieldDeclId == TYPEID(b8) || 
				rawDataTypes.Exists(fieldDeclId))
			{
				memcpy(dstField->GetFieldInstance(dstInstance), srcField->GetFieldInstance(srcInstance), srcField->GetFieldSize());
			}
			else if (fieldDeclId == TYPEID(BinaryBlob)) // Binary blob
			{
				const BinaryBlob& blob = srcField->GetFieldValue<BinaryBlob>(srcInstance);
				dstField->SetFieldValue(dstInstance, blob);
			}
			else if (srcField->IsEventField() && dstField->IsEventField())
			{
				IScriptEvent* srcEvent = srcField->GetFieldEventValue(srcInstance);
				IScriptEvent* dstEvent = dstField->GetFieldEventValue(dstInstance);

				dstEvent->UnbindAll();
				dstEvent->CopyFrom(srcEvent);
			}
			else if (srcField->IsDelegateField() && dstField->IsDelegateField())
			{
				IScriptDelegate* srcDelegate = srcField->GetFieldDelegateValue(srcInstance);
				IScriptDelegate* dstDelegate = dstField->GetFieldDelegateValue(dstInstance);

				dstDelegate->CopyFrom(srcDelegate);
			}
		}
		else if (fieldDeclType->IsStruct())
		{
			auto structType = (StructType*)srcField->GetDeclarationType();
			void* srcStructInstance = srcField->GetFieldInstance(srcInstance);
			void* dstStructInstance = dstField->GetFieldInstance(dstInstance);

			structType->CopyConstructor(srcStructInstance, dstStructInstance);

			for (auto field = structType->GetFirstField(); field != nullptr; field = field->GetNext())
			{
				LoadFromTemplateFieldHelper(originalToClonedObjectMap, field, srcStructInstance, field, dstStructInstance);
			}
		}
		else if (fieldDeclType->IsObject())
		{
			Object* original = nullptr;

			if (srcField->IsStrongRefCounted())
			{
				original = srcField->GetFieldValue<Ref<Object>>(srcInstance).Get();
			}
			else if (srcField->IsWeakRefCounted())
			{
				original = srcField->GetFieldValue<WeakRef<Object>>(srcInstance).Get();
			}
			else
			{
				original = srcField->GetFieldValue<Object*>(srcInstance);
			}

			if (original == nullptr)
			{
				//dstField->SetFieldValue<Object*>(dstInstance, nullptr);
			}
			else
			{
				Object* parent = nullptr;
				Object* valueToSet = nullptr;
				if (originalToClonedObjectMap.KeyExists(original->GetUuid()))
				{
					valueToSet = originalToClonedObjectMap[original->GetUuid()];
				}
				else
				{
					valueToSet = original;
				}

				if (dstField->IsStrongRefCounted())
				{
					dstField->SetFieldValue<Ref<Object>>(dstInstance, valueToSet);
				}
				else if (dstField->IsWeakRefCounted())
				{
					dstField->SetFieldValue<WeakRef<Object>>(dstInstance, valueToSet);
				}
				else
				{
					dstField->SetFieldValue<Object*>(dstInstance, valueToSet);
				}
			}
		}
	}

	void Object::LoadDefaults()
	{
		// Load Base.ini into Object class
		LoadConfig(GetStaticClass<Object>(), "");

		Array<ClassType*> superChain{};
		
		ClassType* super = GetClass();

		while (super != nullptr)
		{
			superChain.Add(super);
			if (super->GetSuperClassCount() > 0)
				super = super->GetSuperClass(0);
			else
				super = nullptr;
		}

		for (int i = superChain.GetSize() - 1; i >= 0; i--)
		{
			if (superChain[i]->GetTypeId() == TYPEID(Object))
				continue;
			
			auto fileName = superChain[i]->GetAttribute("Config").GetStringValue();
			LoadConfig(superChain[i], fileName);
		}
	}

	void Object::ConfigParseStruct(const String& value, void* instance, StructType* structType)
    {
        Array<String> split = value.Split(',');
        for (const String& item : split)
        {
            Array<String> valueSplit = item.Split('=');
            if (valueSplit.GetSize() >= 2)
            {
                String& lhs = valueSplit[0];
                String& rhs = valueSplit[1];

				if (rhs.StartsWith("\""))
					rhs = rhs.GetSubstringView(1);
				if (rhs.EndsWith("\""))
					rhs = rhs.GetSubstringView(0, rhs.GetLength() - 1);
                
				Ptr<FieldType> field = structType->FindField(lhs);
                if (field == nullptr)
                    continue;

				TypeInfo* fieldDeclType = field->GetDeclarationType();
				if (fieldDeclType == nullptr)
					continue;
                
                ConfigParseField(rhs, instance, field);
            }
        }
    }

    void Object::ConfigParseField(const String& value, void* instance, const Ptr<FieldType>& field)
    {
        if (field == nullptr || instance == nullptr)
            return;
        
        TypeId fieldTypeId = field->GetTypeId();
		TypeInfo* fieldDeclType = field->GetDeclarationType();
        
        if (field->IsStringField())
        {
            field->SetFieldValue(instance, value);
        }
        else if (field->GetTypeId() == TYPEID(Name))
        {
            field->SetFieldValue(instance, Name(value));
        }
		else if (fieldTypeId == TYPEID(ClassType))
		{
			ClassType* classType = ClassType::FindClass(value);
			if (classType != nullptr)
				field->SetFieldValue<ClassType*>(this, classType);
		}
		else if (fieldTypeId == TYPEID(StructType))
		{
			StructType* structType = StructType::FindStruct(value);
			if (structType != nullptr)
				field->SetFieldValue<StructType*>(this, structType);
		}
		else if (fieldTypeId == TYPEID(EnumType))
		{
			EnumType* enumType = EnumType::FindEnum(value);
			if (enumType != nullptr)
				field->SetFieldValue<EnumType*>(this, enumType);
		}
		else if (fieldTypeId == TYPEID(SubClassType<>))
		{
			ClassType* classType = ClassType::FindClass(value);
			if (classType != nullptr)
				field->SetFieldValue<SubClassType<>>(this, classType);
		}
        else if (field->GetTypeId() == TYPEID(Uuid))
        {
			field->SetFieldValue(instance, Uuid::FromString(value));
        }
        else if (field->IsIntegerField())
        {
            s64 intValue = 0;
            if (String::TryParse(value, intValue))
            {
                if (fieldTypeId == TYPEID(u8))
                    field->SetFieldValue<u8>(instance, (u8)intValue);
                else if (fieldTypeId == TYPEID(u16))
                    field->SetFieldValue<u16>(instance, (u16)intValue);
                else if (fieldTypeId == TYPEID(u32))
                    field->SetFieldValue<u32>(instance, (u32)intValue);
                else if (fieldTypeId == TYPEID(u64))
                    field->SetFieldValue<u64>(instance, (u64)intValue);
                else if (fieldTypeId == TYPEID(s8))
                    field->SetFieldValue<s8>(instance, (s8)intValue);
                else if (fieldTypeId == TYPEID(s16))
                    field->SetFieldValue<s16>(instance, (s16)intValue);
                else if (fieldTypeId == TYPEID(s32))
                    field->SetFieldValue<s32>(instance, (s32)intValue);
                else if (fieldTypeId == TYPEID(s64))
                    field->SetFieldValue<s64>(instance, (s64)intValue);
            }
        }
        else if (field->IsDecimalField())
        {
            f32 decimalValue = 0;
            if (String::TryParse(value, decimalValue))
            {
                if (fieldTypeId == TYPEID(f32))
                    field->SetFieldValue<f32>(instance, decimalValue);
                else if (fieldTypeId == TYPEID(f64))
                    field->SetFieldValue<f64>(instance, decimalValue);
            }
        }
		else if (field->GetTypeId() == TYPEID(bool))
		{
			bool boolValue = false;
			if (String::TryParse(value, boolValue))
			{
				field->SetFieldValue<bool>(instance, boolValue);
			}
		}
		else if (fieldDeclType != nullptr && fieldDeclType->IsStruct())
		{
			StructType* structType = (StructType*)fieldDeclType;
			void* structInstance = field->GetFieldInstance(instance);

			if (value.StartsWith("(") && value.EndsWith(")"))
				ConfigParseStruct(value.GetSubstringView(1, value.GetLength() - 2), structInstance, structType);
		}
    }


	void Object::OnAfterConstructInternal()
	{
		// The parent should not be pending!
		if (outer != nullptr && EnumHasFlag(outer->objectFlags, OF_SubobjectPending))
		{
			return;
		}
		if (EnumHasFlag(objectFlags, OF_SubobjectPending))
		{
			return;
		}

		for (const auto& subObject : attachedObjects)
		{
			if (!subObject)
				continue;

			if ((subObject->objectFlags & OF_SubobjectPending) != 0)
			{
				subObject->objectFlags &= ~OF_SubobjectPending;
				OnSubobjectAttached(subObject.Get());
				subObject->OnAfterConstructInternal();
			}
		}

		if (!IsDefaultInstance())
		{
			Prefs::Get().LoadPrefs(this);

			OnAfterConstruct();
		}
	}

}

CE_RTTI_CLASS_IMPL(CORE_API, CE, Object)

