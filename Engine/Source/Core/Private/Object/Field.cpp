
#include "Core.h"

namespace CE
{
	FieldType::~FieldType()
	{

	}

	FieldType::FieldType(const String& name, TypeId fieldTypeId, TypeId underlyingTypeId, SIZE_T size, SIZE_T offset,
		String attributes, const TypeInfo* owner, RefType refType, const String& relativePathToParent)
			: TypeInfo(name, attributes)
			, fieldTypeId(fieldTypeId)
			, underlyingTypeId(underlyingTypeId)
			, offset(offset), size(size)
			, owner(const_cast<TypeInfo*>(owner))
			, instanceOwner(const_cast<TypeInfo*>(owner))
			, refType(refType)
			, relativePathToParent(relativePathToParent)
	{
		ConstructInternal();
	}

	Ptr<FieldType> FieldType::Clone(const Ptr<FieldType>& copy)
	{
		return new FieldType(
			copy->GetName().GetString(),
			copy->fieldTypeId,
			copy->underlyingTypeId,
			copy->size,
			copy->offset,
			copy->originalAttributes,
			copy->owner,
			copy->refType,
			copy->relativePathToParent
		);
	}

    void FieldType::ConstructInternal()
    {
        fieldFlags = FIELD_NoFlags;

        if (!HasAttribute("NonSerialized"))
            fieldFlags |= FIELD_Serializable;
        if (HasAttribute("Hidden"))
            fieldFlags |= FIELD_Hidden;
        if (HasAttribute("Config"))
            fieldFlags |= FIELD_Config;
        if (HasAttribute("ReadOnly"))
            fieldFlags |= FIELD_ReadOnly;
        if (HasAttribute("ImportSetting"))
            fieldFlags |= FIELD_ImportSetting;
		if (HasAttribute("Internal"))
			fieldFlags |= FIELD_Internal;
		if (HasAttribute("VisibleAnywhere"))
			fieldFlags |= FIELD_VisibleAnywhere;
		if (HasAttribute("EditAnywhere"))
			fieldFlags |= FIELD_EditAnywhere;
		if (HasAttribute("FusionProperty"))
			fieldFlags |= FIELD_FusionProperty;
		if (HasAttribute("FusionDataProperty"))
			fieldFlags |= FIELD_FusionPropertyData;
		if (HasAttribute("FusionLayoutProperty"))
			fieldFlags |= FIELD_FusionPropertyLayout;
		if (HasAttribute("FusionDataLayoutProperty"))
			fieldFlags |= FIELD_FusionPropertyDataLayout;
    }

	void FieldType::InitializeDefaults(void* instance)
	{
		auto declType = GetDeclarationType();
		auto fieldInstance = GetFieldInstance(instance);
		if (declType == nullptr || fieldInstance == nullptr)
			return;

		if (declType->IsObject())
		{
			if (IsStrongRefCounted())
			{
				(*static_cast<Ref<Object>*>(fieldInstance)) = nullptr;
			}
			else if (IsWeakRefCounted())
			{
				(*static_cast<WeakRef<Object>*>(fieldInstance)) = nullptr;
			}
			else
			{
				memset(fieldInstance, 0, sizeof(SIZE_T)); // Set pointer to nullptr
			}
			return;
		}

		declType->InitializeDefaults(fieldInstance);
	}

	void FieldType::CallDestructor(void* instance)
	{
		// Ideally, this function should NEVER be called!

		auto declType = GetDeclarationType();
		auto fieldInstance = GetFieldInstance(instance);
		if (declType == nullptr || fieldInstance == nullptr)
			return;
		if (declType->IsObject())
		{
			if (IsStrongRefCounted())
			{
				Ref<Object>& object = const_cast<Ref<Object>&>(GetFieldValue<Ref<Object>>(fieldInstance));
				object.~Ref<Object>();
			}
			else if (IsWeakRefCounted())
			{
				WeakRef<Object>& object = const_cast<WeakRef<Object>&>(GetFieldValue<WeakRef<Object>>(fieldInstance));
				object.~WeakRef<Object>();
			}
			else
			{
				if (Object* object = *static_cast<Object**>(fieldInstance))
				{
					object->BeginDestroy();
				}
				memset(fieldInstance, 0, sizeof(SIZE_T)); // Set pointer to nullptr
			}

			return;
		}

		declType->CallDestructor(fieldInstance);
	}

	const CE::Name& FieldType::GetTypeName()
    {
		if (!typeName.IsValid())
		{
			if (owner != nullptr)
				typeName = owner->GetTypeName().GetString() + "::" + name.GetString();
			else
				typeName = name;
		}
		return typeName;
    }

    String FieldType::GetDisplayName()
    {
        String displayAttrib = GetAttribute("DisplayName").GetStringValue();

        if (!displayAttrib.IsEmpty())
            return displayAttrib;

        return TypeInfo::GetDisplayName();
    }

    bool FieldType::IsAssignableTo(TypeId typeId)
    {
        const auto declType = GetDeclarationType();
        
        if (declType == nullptr)
            return false;
        
        if (declType->IsAssignableTo(typeId))
            return true;
        
        return false;
    }

    bool FieldType::IsSerialized() const
    {
        return fieldFlags & FIELD_Serializable;
    }

    bool FieldType::IsHidden() const
    {
        return fieldFlags & FIELD_Hidden;
    }

    bool FieldType::IsReadOnly() const
    {
        return fieldFlags & FIELD_ReadOnly;
    }

    bool FieldType::IsInternal() const
    {
        return fieldFlags & FIELD_Internal;
    }

    bool FieldType::IsFusionProperty()
    {
		return IsFusionBasicProperty() || IsFusionDataProperty() || IsFusionLayoutProperty();
    }

    bool FieldType::IsFusionBasicProperty()
    {
		return EnumHasFlag(fieldFlags, FIELD_FusionProperty);
    }

    bool FieldType::IsFusionDataProperty()
    {
		return EnumHasFlag(fieldFlags, FIELD_FusionPropertyData);
    }

    bool FieldType::IsFusionDataLayoutProperty()
    {
		return EnumHasFlag(fieldFlags, FIELD_FusionPropertyDataLayout);
    }

    bool FieldType::IsFusionLayoutProperty()
    {
		return EnumHasFlag(fieldFlags, FIELD_FusionPropertyLayout);
    }

    bool FieldType::IsEditAnywhere() const
    {
		return fieldFlags & FIELD_EditAnywhere;
    }

    bool FieldType::IsVisibleAnywhere() const
    {
		return fieldFlags & FIELD_VisibleAnywhere;
    }

    TypeInfo* FieldType::GetOwnerType()
	{
		return const_cast<TypeInfo*>(owner);
	}

	TypeInfo* FieldType::GetInstanceOwnerType()
	{
		return const_cast<TypeInfo*>(instanceOwner);
	}

	bool FieldType::HasCustomPODSerialization()
	{
		return IsPODField() && GetDeclarationType()->HasCustomPODSerialization();
	}

	bool FieldType::IsPODField()
	{
		if (GetDeclarationType() == nullptr)
			return false;
		return GetDeclarationType()->IsPOD();
	}

	bool FieldType::IsArrayField() const
    {
        return GetDeclarationTypeId() == TYPEID(Array<u8>);
    }

    bool FieldType::IsStringField() const
    {
        return GetDeclarationTypeId() == TYPEID(String);
    }

	bool FieldType::IsTypeInfoField()
	{
		if (GetDeclarationType() == nullptr)
			return false;
		return GetDeclarationType()->IsTypeInfo();
	}

    bool FieldType::IsIntegerField() const
    {
        return fieldTypeId == TYPEID(u8) || fieldTypeId == TYPEID(u16) || fieldTypeId == TYPEID(u32) || fieldTypeId == TYPEID(u64) ||
            fieldTypeId == TYPEID(s8) || fieldTypeId == TYPEID(s16) || fieldTypeId == TYPEID(s32) || fieldTypeId == TYPEID(s64);
    }

    bool FieldType::IsDecimalField() const
    {
        return fieldTypeId == TYPEID(f32) || fieldTypeId == TYPEID(f64);
    }

	bool FieldType::IsEnumField()
	{
		auto declType = GetDeclarationType();
		if (declType == nullptr)
			return false;
		return declType->IsEnum();
	}

	bool FieldType::IsEnumFlagsField()
	{
		auto declType = GetDeclarationType();
		if (declType == nullptr)
			return false;
		return declType->IsEnum() && ((EnumType*)declType)->IsFlagsEnum();
	}

	bool FieldType::IsEnumArrayField()
	{
		if (!IsArrayField())
			return false;
		auto underlyingDeclType = GetUnderlyingType();
		if (underlyingDeclType == nullptr)
			return false;
		return underlyingDeclType->IsEnum();
	}

	bool FieldType::IsEventField() const
	{
		return GetDeclarationTypeId() == Internal::GetScriptEventTypeId();
	}

	bool FieldType::IsDelegateField() const
	{
		return GetDeclarationTypeId() == Internal::GetScriptDelegateTypeId();
	}

	bool FieldType::IsPropertyBindingField() const
	{
		return GetDeclarationTypeId() == Internal::GetPropertyBindingTypeId();
	}

	bool FieldType::IsObjectField() const
    {
		TypeInfo* declType = GetDeclarationType();
		return declType != nullptr && declType->IsClass();
    }

	bool FieldType::IsStructField()
	{
		TypeInfo* declType = GetDeclarationType();
		return declType != nullptr && declType->IsStruct();
	}

	bool FieldType::IsRefCounted() const
	{
		return IsStrongRefCounted() || IsWeakRefCounted();
	}

	bool FieldType::IsStrongRefCounted() const
	{
		return refType == RefType::Strong;
	}

	bool FieldType::IsWeakRefCounted() const
	{
		return refType == RefType::Weak;
	}

	TypeInfo* FieldType::GetUnderlyingType()
	{
		if (underlyingTypeInfo == nullptr && underlyingTypeId != 0)
		{
			underlyingTypeInfo = GetTypeInfo(underlyingTypeId);
		}
		return underlyingTypeInfo;
	}

	TypeInfo* FieldType::GetDeclarationType() const
    {
		if (declarationType == nullptr)
		{
			declarationType = GetTypeInfo(fieldTypeId);
		}
		return declarationType;
    }

	String FieldType::GetFieldValueAsString(void* instance)
	{
		if (GetDeclarationType() == nullptr)
			return "";

		auto declTypeId = GetDeclarationTypeId();
		auto declType = GetDeclarationType();

		if (declType->IsPOD())
		{
			if (declTypeId == TYPEID(String))
			{
				return GetFieldValue<String>(instance);
			}
			else if (declTypeId == TYPEID(Name))
			{
				return GetFieldValue<Name>(instance).GetString();
			}
			else if (declTypeId == TYPEID(IO::Path))
			{
				return GetFieldValue<IO::Path>(instance).GetString();
			}
			else if (fieldTypeId == TYPEID(Uuid))
			{
				return String::Format("{}", GetFieldValue<Uuid>(instance));
			}
			else if (fieldTypeId == TYPEID(c8))
			{
				return String::Format("{}", (int)GetFieldValue<c8>(instance));
			}
			else if (fieldTypeId == TYPEID(c16))
			{
				return String::Format("{}", (int)GetFieldValue<c16>(instance));
			}
			else if (fieldTypeId == TYPEID(u8))
			{
				return String::Format("{}", (int)GetFieldValue<u8>(instance));
			}
			else if (fieldTypeId == TYPEID(s8))
			{
				return String::Format("{}", (int)GetFieldValue<s8>(instance));
			}
			else if (fieldTypeId == TYPEID(u16))
			{
				return String::Format("{}", GetFieldValue<u16>(instance));
			}
			else if (fieldTypeId == TYPEID(s16))
			{
				return String::Format("{}", GetFieldValue<s16>(instance));
			}
			else if (fieldTypeId == TYPEID(u32))
			{
				return String::Format("{}", GetFieldValue<u32>(instance));
			}
			else if (fieldTypeId == TYPEID(s32))
			{
				return String::Format("{}", GetFieldValue<s32>(instance));
			}
			else if (fieldTypeId == TYPEID(u64))
			{
				return String::Format("{}", GetFieldValue<u64>(instance));
			}
			else if (fieldTypeId == TYPEID(s64))
			{
				return String::Format("{}", GetFieldValue<s64>(instance));
			}
			else if (fieldTypeId == TYPEID(bool))
			{
				return GetFieldValue<bool>(instance) ? "true" : "false";
			}
			else if (fieldTypeId == TYPEID(f32))
			{
				return String::Format("{}", GetFieldValue<f32>(instance));
			}
			else if (fieldTypeId == TYPEID(f64))
			{
				return String::Format("{}", GetFieldValue<f64>(instance));
			}
			else if (IsArrayField())
			{
				auto underlyingType = this->GetUnderlyingType();
				if (underlyingType != nullptr && underlyingType->IsPOD() && !underlyingType->IsArrayType())
				{
					auto arrayList = GetArrayFieldListPtr(instance);
					u32 arraySize = arrayList.GetSize();
					void* arrayFieldInstance = GetFieldInstance(instance);

					if (arraySize > 0)
					{
						String result = "[";
						for (int i = 0; i < arraySize; i++)
						{
							auto str = arrayList[i]->GetFieldValueAsString(arrayFieldInstance);
							if (result.NotEmpty())
								result += ",";
							result += str;
						}
						result += "]";
						return result;
					}
				}
			}
			else if (fieldTypeId == TYPEID(SubClassType<>))
			{
				ClassType* type = GetFieldValue<SubClassType<Object>>(instance);
				if (type == nullptr)
					return "";
				return type->GetTypeName().GetString();
			}
		}
		else if (declType->IsEnum())
		{
			EnumType* enumType = (EnumType*)declType;
			EnumConstant* constant = enumType->FindConstantWithValue(GetFieldEnumValue(instance));
			if (constant == nullptr)
				return "";
			return constant->GetName().GetString();
		}
		else if (fieldTypeId == TYPEID(ClassType))
		{
			ClassType* type = GetFieldValue<ClassType*>(instance);
			if (type == nullptr)
				return "";
			return type->GetTypeName().GetString();
		}
		else if (fieldTypeId == TYPEID(StructType))
		{
			StructType* type = GetFieldValue<StructType*>(instance);
			if (type == nullptr)
				return "";
			return type->GetTypeName().GetString();
		}
		else if (fieldTypeId == TYPEID(EnumType))
		{
			EnumType* type = GetFieldValue<EnumType*>(instance);
			if (type == nullptr)
				return "";
			return type->GetTypeName().GetString();
		}

		return "";
	}

	Ref<Object> FieldType::GetFieldObjectValue(void* instance)
	{
		if (!IsObjectField())
			return nullptr;

		if (IsStrongRefCounted())
		{
			return GetFieldValue<Ref<Object>>(instance);
		}
		if (IsWeakRefCounted())
		{
			return GetFieldValue<WeakRef<Object>>(instance).Lock();
		}

		return GetFieldValue<Object*>(instance);
	}

	f64 FieldType::GetNumericFieldValue(void* instance)
	{
		if (GetDeclarationType() == nullptr)
			return 0;;

		auto declTypeId = GetDeclarationTypeId();
		auto declType = GetDeclarationType();

		if (declType->IsPOD())
		{
			if (declTypeId == TYPEID(String))
			{
				String string = GetFieldValue<String>(instance);
				f64 value = 0;
				if (String::TryParse(string, value))
				{
					return value;
				}
			}
			else if (declTypeId == TYPEID(Name))
			{
				String string = GetFieldValue<Name>(instance).GetString();
				f64 value = 0;
				if (String::TryParse(string, value))
				{
					return value;
				}
			}
			else if (fieldTypeId == TYPEID(c8))
			{
				return (int)GetFieldValue<c8>(instance);
			}
			else if (fieldTypeId == TYPEID(c16))
			{
				return (int)GetFieldValue<c16>(instance);
			}
			else if (fieldTypeId == TYPEID(u8))
			{
				return (int)GetFieldValue<u8>(instance);
			}
			else if (fieldTypeId == TYPEID(s8))
			{
				return (int)GetFieldValue<s8>(instance);
			}
			else if (fieldTypeId == TYPEID(u16))
			{
				return GetFieldValue<u16>(instance);
			}
			else if (fieldTypeId == TYPEID(s16))
			{
				return GetFieldValue<s16>(instance);
			}
			else if (fieldTypeId == TYPEID(u32))
			{
				return GetFieldValue<u32>(instance);
			}
			else if (fieldTypeId == TYPEID(s32))
			{
				return GetFieldValue<s32>(instance);
			}
			else if (fieldTypeId == TYPEID(u64))
			{
				return GetFieldValue<u64>(instance);
			}
			else if (fieldTypeId == TYPEID(s64))
			{
				return GetFieldValue<s64>(instance);
			}
			else if (fieldTypeId == TYPEID(bool))
			{
				return GetFieldValue<bool>(instance) ? 1 : 0;
			}
			else if (fieldTypeId == TYPEID(f32))
			{
				return GetFieldValue<f32>(instance);
			}
			else if (fieldTypeId == TYPEID(f64))
			{
				return GetFieldValue<f64>(instance);
			}
		}

		return 0;
	}

	void FieldType::SetFieldObjectValue(void* instance, const Ref<Object>& object)
	{
		if (IsReadOnly() || !IsObjectField())
			return;

		if (IsStrongRefCounted())
		{
			SetFieldValue<Ref<Object>>(instance, object);
		}
		else if (IsWeakRefCounted())
		{
			SetFieldValue<WeakRef<Object>>(instance, object);
		}
		else
		{
			SetFieldValue<Object*>(instance, object.Get());
		}
	}

	bool FieldType::CopyTo(void* srcInstance, const Ptr<FieldType>& destField, void* destInstance)
	{
		if (destField == nullptr || destField->fieldTypeId != fieldTypeId || srcInstance == nullptr || destInstance == nullptr)
			return false;
		if (destField->IsReadOnly() || GetDeclarationType() == nullptr)
			return false;

		if (GetDeclarationType()->IsPOD())
		{
			if (fieldTypeId == TYPEID(String))
			{
				const String& value = GetFieldValue<String>(srcInstance);
				destField->SetFieldValue(destInstance, value);
			}
			else if (fieldTypeId == TYPEID(Name))
			{
				const Name& value = GetFieldValue<Name>(srcInstance);
				destField->SetFieldValue(destInstance, value);
			}
			else if (fieldTypeId == TYPEID(IO::Path))
			{
				const IO::Path& value = GetFieldValue<IO::Path>(srcInstance);
				destField->SetFieldValue(destInstance, value);
			}
			else if (fieldTypeId == TYPEID(Uuid))
			{
				const Uuid& value = GetFieldValue<Uuid>(srcInstance);
				destField->SetFieldValue(destInstance, value);
			}
			else if (IsArrayField()) // Array
			{
				auto underlyingType = this->GetUnderlyingType();
				if (underlyingType != nullptr && underlyingType == destField->GetUnderlyingType())
				{
					u32 srcArraySize = this->GetArraySize(srcInstance);
					destField->ResizeArray(destInstance, srcArraySize);

					if (srcArraySize > 0)
					{
						Array<Ptr<FieldType>> arrayFields = this->GetArrayFieldListPtr(srcInstance);
						u8* srcArrayInstance = const_cast<u8*>(&this->GetFieldValue<Array<u8>>(srcInstance)[0]);
						u8* destArrayInstance = const_cast<u8*>(&this->GetFieldValue<Array<u8>>(destInstance)[0]);

						for (int i = 0; i < arrayFields.GetSize(); i++)
						{
							arrayFields[i]->CopyTo(srcArrayInstance, arrayFields[i], destArrayInstance);
						}
					}
				}
				else
				{
					return false;
				}
			}
			else if (IsIntegerField() || IsDecimalField() || fieldTypeId == TYPEID(bool)) // Numeric types
			{
				memcpy(destField->GetFieldInstance(destInstance), GetFieldInstance(srcInstance), this->GetFieldSize());
			}
			else if (fieldTypeId == TYPEID(BinaryBlob)) // Binary blob
			{
				const BinaryBlob& blob = GetFieldValue<BinaryBlob>(srcInstance);
				destField->SetFieldValue(destInstance, blob);
			}
			else
			{
				return false;
			}
		}
		else if (GetDeclarationType()->IsStruct())
		{
			auto structType = (StructType*)GetDeclarationType();
			void* srcStructInstance = this->GetFieldInstance(srcInstance);
			void* dstStructInstance = destField->GetFieldInstance(destInstance);

			for (auto field = structType->GetFirstField(); field != nullptr; field = field->GetNext())
			{
				field->CopyTo(srcStructInstance, field, dstStructInstance);
			}
		}
		else if (GetDeclarationType()->IsClass())
		{
			Object* objectPtr = this->GetFieldValue<Object*>(srcInstance);
			destField->SetFieldValue(destInstance, objectPtr);
		}
		else if (GetDeclarationTypeId() == TYPEID(ClassType))
		{
			ClassType* type = this->GetFieldValue<ClassType*>(srcInstance);
			destField->SetFieldValue(destInstance, type);
		}
		else if (GetDeclarationTypeId() == TYPEID(StructType))
		{
			StructType* type = this->GetFieldValue<StructType*>(srcInstance);
			destField->SetFieldValue(destInstance, type);
		}
		else if (GetDeclarationTypeId() == TYPEID(SubClassType<Object>))
		{
			const SubClassType<Object>& type = this->GetFieldValue<SubClassType<Object>>(srcInstance);
			destField->SetFieldValue(destInstance, type);
		}
		else
		{
			return false;
		}

		return true;
	}

	s64 FieldType::GetFieldEnumValue(void* instance)
    {
        if (!GetDeclarationType()->IsEnum())
            return 0;

        auto enumType = GetDeclarationType();

        if (enumType->GetSize() == sizeof(u8))
        {
            return GetFieldValue<u8>(instance);
        }
        else if (enumType->GetSize() == sizeof(u16))
        {
            return GetFieldValue<u16>(instance);
        }
        else if (enumType->GetSize() == sizeof(u32))
        {
            return GetFieldValue<u32>(instance);
        }
        else if (enumType->GetSize() == sizeof(u64))
        {
            return GetFieldValue<u64>(instance);
        }

        return 0;
    }

    void FieldType::SetFieldEnumValue(void* instance, s64 value)
    {
        if (!GetDeclarationType()->IsEnum())
            return;
		if (IsReadOnly())
			return;

		ForceSetFieldEnumValue(instance, value);

		if (instanceOwner && instanceOwner->IsObject())
		{
			NotifyObjectFieldUpdate((Object*)instance);
		}
    }

	void FieldType::ForceSetFieldEnumValue(void* instance, s64 value)
	{
		auto enumType = GetDeclarationType();

		if (enumType->GetSize() == sizeof(u8))
		{
			SetFieldValue<u8>(instance, value);
		}
		else if (enumType->GetSize() == sizeof(u16))
		{
			SetFieldValue<u16>(instance, value);
		}
		else if (enumType->GetSize() == sizeof(u32))
		{
			SetFieldValue<u32>(instance, value);
		}
		else if (enumType->GetSize() == sizeof(u64))
		{
			SetFieldValue<u64>(instance, value);
		}
	}

	u32 FieldType::GetArraySize(void* instance)
	{
		if (!IsArrayField())
			return 0;

		const auto& array = GetFieldValue<Array<u8>>(instance);
		TypeId underlyingTypeId = GetUnderlyingTypeId();
		if (underlyingTypeId == 0)
			return 0;

		TypeInfo* underlyingType = GetUnderlyingType();
		if (underlyingType == nullptr)
			return 0;

		auto underlyingTypeSize = underlyingType->GetSize();

		if (underlyingType->IsClass())
		{
			underlyingTypeSize = sizeof(Object*); // classes are always stored as pointers

			if (IsStrongRefCounted())
			{
				underlyingTypeSize = sizeof(Ref<>);
			}
			else if (IsWeakRefCounted())
			{
				underlyingTypeSize = sizeof(WeakRef<>);
			}
		}
		if (underlyingType->IsTypeInfo())
		{
			underlyingTypeSize = sizeof(TypeInfo*); // Reflection types are always stored as raw pointers
		}

		return array.GetSize() / underlyingTypeSize;
	}

	void FieldType::ResizeArray(void* instance, u32 numElements)
	{
		if (!IsArrayField() || instance == nullptr)
			return;

		auto curNumElements = GetArraySize(instance);

		Array<u8>& array = const_cast<Array<u8>&>(GetFieldValue<Array<u8>>(instance));
		TypeId underlyingTypeId = GetUnderlyingTypeId();
		if (underlyingTypeId == 0)
			return;

		TypeInfo* underlyingType = GetUnderlyingType();
		if (underlyingType == nullptr)
			return;

		auto underlyingTypeSize = underlyingType->GetSize();

		if (underlyingType->IsClass())
		{
			underlyingTypeSize = sizeof(Object*); // classes are always stored as pointers

			if (IsStrongRefCounted())
			{
				underlyingTypeSize = sizeof(Ref<>);
			}
			else if (IsWeakRefCounted())
			{
				underlyingTypeSize = sizeof(WeakRef<>);
			}

			array.Resize(numElements * underlyingTypeSize, 0);
		}
		else
		{
			for (int i = numElements; i < curNumElements; i++) // Call destructor on elements that will be removed
			{
				void* elementInstance = array.GetData() + underlyingTypeSize * i;
				underlyingType->CallDestructor(elementInstance);
			}

			array.Resize(numElements * underlyingTypeSize, 0);

			for (int i = curNumElements; i < numElements; i++) // Initialize newly added elements if exists
			{
				void* elementInstance = array.GetData() + underlyingTypeSize * i;
				underlyingType->InitializeDefaults(elementInstance);
			}
		}
	}

	void FieldType::InsertArrayElement(void* instance)
	{
		if (!IsArrayField() || instance == nullptr)
			return;

		auto arraySize = GetArraySize(instance);
		InsertArrayElement(instance, arraySize);
	}

	void FieldType::InsertArrayElement(void* instance, u32 insertPosition)
	{
		if (!IsArrayField() || instance == nullptr)
			return;

		auto arraySize = GetArraySize(instance);

		if (insertPosition == NumericLimits<u32>::Max())
		{
			insertPosition = arraySize;
		}
		if (insertPosition > arraySize)
			return;

		Array<u8>& array = const_cast<Array<u8>&>(GetFieldValue<Array<u8>>(instance));
		TypeId underlyingTypeId = GetUnderlyingTypeId();
		if (underlyingTypeId == 0)
			return;

		TypeInfo* underlyingType = GetUnderlyingType();
		if (underlyingType == nullptr)
			return;

		auto underlyingTypeSize = underlyingType->GetSize();

		if (underlyingType->IsClass())
		{
			underlyingTypeSize = sizeof(Object*); // classes are always stored as pointers

			if (IsStrongRefCounted())
			{
				underlyingTypeSize = sizeof(Ref<>);
				Array<Ref<Object>>& refArray = const_cast<Array<Ref<Object>>&>(GetFieldValue<Array<Ref<Object>>>(instance));
				refArray.InsertAt(insertPosition, Ref<Object>(nullptr));

				return;
			}
			else if (IsWeakRefCounted())
			{
				underlyingTypeSize = sizeof(WeakRef<>);
				Array<WeakRef<Object>>& refArray = const_cast<Array<WeakRef<Object>>&>(GetFieldValue<Array<WeakRef<Object>>>(instance));
				refArray.InsertAt(insertPosition, WeakRef<Object>(nullptr));

				return;
			}
		}

		auto insertBytePos = insertPosition * underlyingTypeSize;
		
		Array<u8> insertionValues = Array<u8>(underlyingTypeSize, 0);
		array.InsertRange(insertBytePos, insertionValues); // Insert an array of zeroes

		if (underlyingType->IsClass()) // Do NOT call InitializeDefaults on a classes (aka Objects). They're just null pointers by default.
			return;

		void* elementInstance = &array[insertBytePos];
		underlyingType->InitializeDefaults(elementInstance);
	}

	void FieldType::DeleteArrayElement(void* instance, u32 deletePosition)
	{
		if (!IsArrayField() || instance == nullptr)
			return;

		auto curNumElements = GetArraySize(instance);

		if (curNumElements == 0 || deletePosition >= curNumElements)
			return;

		Array<u8>& array = const_cast<Array<u8>&>(GetFieldValue<Array<u8>>(instance));
		TypeId underlyingTypeId = GetUnderlyingTypeId();
		if (underlyingTypeId == 0)
			return;

		TypeInfo* underlyingType = GetUnderlyingType();
		if (underlyingType == nullptr)
			return;

		auto underlyingTypeSize = underlyingType->GetSize();
		u32 deletionBytePos = deletePosition * underlyingTypeSize;

		if (underlyingType->IsClass())
		{
			underlyingTypeSize = sizeof(Object*); // classes are always stored as pointers

			if (IsStrongRefCounted())
			{
				underlyingTypeSize = sizeof(Ref<>);
				Array<Ref<Object>>& refArray = (Array<Ref<Object>>&)array;
				refArray.RemoveAt(deletePosition);

				return;
			}
			else if (IsWeakRefCounted())
			{
				underlyingTypeSize = sizeof(WeakRef<>);
				Array<WeakRef<Object>>& refArray = (Array<WeakRef<Object>>&)array;
				refArray.RemoveAt(deletePosition);

				return;
			}

			deletionBytePos = deletePosition * underlyingTypeSize;
		}
		else
		{
			void* elementInstance = &array[deletionBytePos];
			underlyingType->CallDestructor(elementInstance);
		}

		for (int i = 0; i < underlyingTypeSize; i++) // Remove bytes from array
		{
			array.RemoveAt(deletionBytePos);
		}
	}

	Array<Ptr<FieldType>> FieldType::GetArrayFieldListPtr(void* instance)
	{
		if (!IsArrayField())
			return {};

		u32 arraySize = GetArraySize(instance);
		if (arraySize == 0)
			return {};

		TypeInfo* underlyingType = GetUnderlyingType();
		if (underlyingType == nullptr)
			return {};

		auto underlyingTypeSize = underlyingType->GetSize();

		if (underlyingType->IsClass())
		{
			underlyingTypeSize = sizeof(Object*); // Classes are always stored as object pointers

			if (IsStrongRefCounted())
			{
				underlyingTypeSize = sizeof(Ref<>);
			}
			else if (IsWeakRefCounted())
			{
				underlyingTypeSize = sizeof(WeakRef<>);
			}
		}

		Array<Ptr<FieldType>> array{};

		for (int i = 0; i < arraySize; i++)
		{
			// TODO: We need to add ref counting Ptr<> to "owner: this" part too.
			array.Add(new FieldType(name.GetString() + "[" + i + "]", underlyingType->GetTypeId(),
				0, underlyingTypeSize, i * underlyingTypeSize, "", this, refType));
		}

		return array;
	}

	Ptr<FieldType> FieldType::GetArrayFieldElementPtr(void* instance, u32 position)
	{
		if (!IsArrayField())
			return {};

		u32 arraySize = GetArraySize(instance);
		if (arraySize == 0)
			return {};

		TypeInfo* underlyingType = GetUnderlyingType();
		if (underlyingType == nullptr)
			return {};

		int underlyingTypeSize = underlyingType->GetSize();

		if (underlyingType->IsClass())
		{
			underlyingTypeSize = sizeof(Object*); // Classes are always stored as object pointers

			if (IsStrongRefCounted())
			{
				underlyingTypeSize = sizeof(Ref<>);
			}
			else if (IsWeakRefCounted())
			{
				underlyingTypeSize = sizeof(WeakRef<>);
			}
		}

		int i = position;

		return new FieldType(name.GetString() + "[" + i + "]", underlyingType->GetTypeId(),
			0, underlyingTypeSize, i * underlyingTypeSize, "", this, refType);
	}

	void* FieldType::GetArrayInstance(void* instance)
	{
		if (!IsArrayField())
			return nullptr;

		const Array<u8>& array = GetFieldValue<Array<u8>>(instance);

		return (void*)array.GetData();
	}

	void FieldType::NotifyObjectFieldUpdate(Object* instance)
	{
		
	}
}

CE_RTTI_TYPEINFO_IMPL(CE, FieldType)
