#include "CoreMinimal.h"

namespace CE
{
	HashMap<Name, Array<ClassType*>> Prefs::prefsClasses{};

	void Prefs::ClearAllPrefs()
	{
		prefsJson.Clear();
	}

	void Prefs::LoadPrefsJson()
	{
		ZoneScoped;

		prefsJson.Clear();

		for (const auto& [prefsName, classes] : prefsClasses)
		{
			if (!prefsName.IsValid())
				continue;

			IO::Path appDataPath{};
#if PAL_TRAIT_BUILD_EDITOR
			if (prefsName.GetString().StartsWith("Editor"))
			{
				appDataPath = PlatformDirectories::GetEditorAppDataDir();
			}
			else
#endif
			{
				appDataPath = PlatformDirectories::GetAppDataDir();
			}

			if (!appDataPath.Exists())
				IO::Path::CreateDirectories(appDataPath);

			auto prefsPath = appDataPath / String::Format("{}Prefs.json", prefsName.GetString());

			if (prefsPath.Exists())
			{
				FileStream reader = FileStream(prefsPath, Stream::Permissions::ReadOnly);
				JsonSerializer::Deserialize2(&reader, prefsJson[prefsName]);
			}
			else
			{
				prefsJson[prefsName] = JObject();
			}
		}
	}

	void Prefs::SavePrefsJson()
	{
		ZoneScoped;

		for (const auto& [prefsName, classes] : prefsClasses)
		{
			if (!prefsName.IsValid())
				continue;

			IO::Path appDataPath{};
#if PAL_TRAIT_BUILD_EDITOR
			if (prefsName.GetString().StartsWith("Editor"))
			{
				appDataPath = PlatformDirectories::GetEditorAppDataDir();
			}
			else
#endif
			{
				appDataPath = PlatformDirectories::GetAppDataDir();
			}

			if (!appDataPath.Exists())
				IO::Path::CreateDirectories(appDataPath);

			auto prefsPath = appDataPath / String::Format("{}Prefs.json", prefsName.GetString());

			FileStream writer = FileStream(prefsPath, Stream::Permissions::WriteOnly);

			if (prefsJson.KeyExists(prefsName))
			{
				const JValue& json = prefsJson[prefsName];

				if (!json.IsObjectValue())
				{
					JsonSerializer::Serialize2(&writer, JObject());
				}
				else
				{
					JsonSerializer::Serialize2(&writer, json);
				}
			}
		}
	}

	void Prefs::LoadPrefs(Object* object)
	{
		ZoneScoped;

		if (!object)
			return;

		auto clazz = object->GetClass();
		if (!clazz || !clazz->HasAttribute("Prefs"))
			return;

		Name prefsName = clazz->GetAttribute("Prefs").GetStringValue();
		if (!prefsName.IsValid())
			return;

		if (!prefsJson.KeyExists(prefsName))
		{
			return;
		}

		Name classTypeName = clazz->GetTypeName();
		if (!prefsJson[prefsName].KeyExists(classTypeName.GetString()) ||
			!prefsJson[prefsName][classTypeName.GetString()].IsObjectValue())
		{
			return;
		}

		JValue& root = prefsJson[prefsName][classTypeName.GetString()];

		for (Ptr<FieldType> field = clazz->GetFirstField(); field != nullptr; field = field->GetNext())
		{
			if (!field->HasAttribute("Prefs"))
				continue;

			DeserializeField(field, object, root);
		}
	}

	void Prefs::SavePrefs(Object* object)
	{
		ZoneScoped;

		if (!object)
			return;

		auto clazz = object->GetClass();
		if (!clazz || !clazz->HasAttribute("Prefs"))
			return;

		Name prefsName = clazz->GetAttribute("Prefs").GetStringValue();
		if (!prefsName.IsValid())
			return;

		if (!prefsJson.KeyExists(prefsName))
		{
			prefsJson[prefsName] = JObject();
		}
		
		Name classTypeName = clazz->GetTypeName();
		prefsJson[prefsName][classTypeName.GetString()] = JObject();
		JValue& root = prefsJson[prefsName][classTypeName.GetString()];

		for (Ptr<FieldType> field = clazz->GetFirstField(); field != nullptr; field = field->GetNext())
		{
			if (!field->HasAttribute("Prefs"))
				continue;

			SerializeField(field, object, root);
		}
	}
	
	void Prefs::SerializeField(const Ptr<FieldType>& field, void* instance, JValue& parent)
	{
		ZoneScoped;

		const String& fieldName = field->GetName().GetString();
		TypeId fieldTypeId = field->GetDeclarationTypeId();

		if (field->IsNumericField())
		{
			if (parent.IsObjectValue())
				parent.GetObjectValue()[fieldName] = field->GetNumericFieldValue(instance);
			else if (parent.IsArrayValue())
				parent.GetArrayValue().Add(field->GetNumericFieldValue(instance));
		}
		else if (fieldTypeId == TYPEID(String) || fieldTypeId == TYPEID(Name))
		{
			if (parent.IsObjectValue())
				parent.GetObjectValue()[fieldName] = field->GetFieldValueAsString(instance);
			else if (parent.IsArrayValue())
				parent.GetArrayValue().Add(field->GetFieldValueAsString(instance));
		}
		else if (fieldTypeId == TYPEID(Vec2))
		{
			Vec2 value = field->GetFieldValue<Vec2>(instance);
			if (parent.IsObjectValue())
				parent.GetObjectValue()[fieldName] = JArray({ value.x, value.y });
			else if (parent.IsArrayValue())
				parent.GetArrayValue().Add(JArray({ value.x, value.y }));
		}
		else if (fieldTypeId == TYPEID(Vec3))
		{
			Vec3 value = field->GetFieldValue<Vec3>(instance);
			if (parent.IsObjectValue())
				parent.GetObjectValue()[fieldName] = JArray({ value.x, value.y, value.z });
			else if (parent.IsArrayValue())
				parent.GetArrayValue().Add(JArray({ value.x, value.y, value.z }));
		}
		else if (fieldTypeId == TYPEID(Vec4))
		{
			Vec4 value = field->GetFieldValue<Vec4>(instance);
			if (parent.IsObjectValue())
				parent.GetObjectValue()[fieldName] = JArray({ value.x, value.y, value.z, value.w });
			else if (parent.IsArrayValue())
				parent.GetArrayValue().Add(JArray({ value.x, value.y, value.z, value.w }));
		}
		else if (fieldTypeId == TYPEID(Color))
		{
			Color value = field->GetFieldValue<Color>(instance);
			if (parent.IsObjectValue())
				parent.GetObjectValue()[fieldName] = JArray({ value.r, value.g, value.b, value.a });
			else if (parent.IsArrayValue())
				parent.GetArrayValue().Add(JArray({ value.r, value.g, value.b, value.a }));
		}
		else if (field->IsArrayField() && field->GetUnderlyingType() != nullptr && 
			(field->GetUnderlyingType()->IsStruct() || field->GetUnderlyingType()->IsPOD()))
		{
			Array<u8>& array = const_cast<Array<u8>&>(field->GetFieldValue<Array<u8>>(instance));

			Array<Ptr<FieldType>> fieldList = field->GetArrayFieldListPtr(instance);
			if (parent.IsObjectValue())
				parent.GetObjectValue()[fieldName] = JArray();
			else if (parent.IsArrayValue())
				parent.GetArrayValue().Add(JArray());
			else
				return;

			JValue& arrayParent = parent.IsObjectValue() ? parent[fieldName] : parent.GetArrayValue().Top();

			if (!fieldList.IsEmpty())
			{
				arrayParent.GetArrayValue().Reserve(fieldList.GetSize());
				void* arrayInstance = &array[0];

				for (int i = 0; i < fieldList.GetSize(); ++i)
				{
					SerializeField(fieldList[i], arrayInstance, arrayParent);
				}
			}
		}
		else if (field->IsStructField() && field->GetDeclarationType() != nullptr)
		{
			void* structInstance = field->GetFieldInstance(instance);
			StructType* structType = (StructType*)field->GetDeclarationType();

			if (parent.IsObjectValue())
				parent.GetObjectValue()[fieldName] = JObject();
			else if (parent.IsArrayValue())
				parent.GetArrayValue().Add(JObject());
			else
				return;

			JValue& objectParent = parent.IsObjectValue() ? parent[fieldName] : parent.GetArrayValue().Top();

			for (Ptr<FieldType> structField = structType->GetFirstField(); structField != nullptr; structField = structField->GetNext())
			{
				SerializeField(structField, structInstance, objectParent);
			}
		}
	}

#define DESERIALIZE_FIELD(type) if (fieldTypeId == TYPEID(type))\
	field->SetFieldValue<type>(instance, static_cast<type>(value));

	void Prefs::DeserializeField(const Ptr<FieldType>& field, void* instance, JValue& parent, int index)
	{
		ZoneScoped;

		const String& fieldName = field->GetName().GetString();
		TypeId fieldTypeId = field->GetDeclarationTypeId();

		if (field->IsNumericField())
		{
			f64 value = 0.0f;
			if (parent.IsObjectValue())
				value = parent.GetObjectValue()[fieldName].GetNumberValue();
			else if (parent.IsArrayValue())
				value = parent.GetArrayValue()[index].GetNumberValue();
			else
				return;

			DESERIALIZE_FIELD(u8) else
			DESERIALIZE_FIELD(u16) else
			DESERIALIZE_FIELD(u32) else
			DESERIALIZE_FIELD(u64) else
			DESERIALIZE_FIELD(s8) else
			DESERIALIZE_FIELD(s16) else
			DESERIALIZE_FIELD(s32) else
			DESERIALIZE_FIELD(s64) else
			DESERIALIZE_FIELD(f32) else
			DESERIALIZE_FIELD(f64)
		}
		else if (fieldTypeId == TYPEID(String) || fieldTypeId == TYPEID(Name))
		{
			String value = "";
			if (parent.IsObjectValue())
				value = parent.GetObjectValue()[fieldName].GetStringValue();
			else if (parent.IsArrayValue())
				value = parent.GetArrayValue()[index].GetStringValue();
			else
				return;

			DESERIALIZE_FIELD(String) else
			DESERIALIZE_FIELD(Name)

		}
		else if (fieldTypeId == TYPEID(Vec2))
		{
			if (!parent.IsObjectValue() && !parent.IsArrayValue())
				return;

			JArray& arrayValue = parent.IsObjectValue() ? parent.GetObjectValue()[fieldName].GetArrayValue() : parent.GetArrayValue()[index].GetArrayValue();

			Vec2 value = Vec2();
			value.x = arrayValue[0].GetNumberValue();
			value.y = arrayValue[1].GetNumberValue();

			field->SetFieldValue<Vec2>(instance, value);
		}
		else if (fieldTypeId == TYPEID(Vec3))
		{
			if (!parent.IsObjectValue() && !parent.IsArrayValue())
				return;

			JArray& arrayValue = parent.IsObjectValue() ? parent.GetObjectValue()[fieldName].GetArrayValue() : parent.GetArrayValue()[index].GetArrayValue();

			Vec3 value = Vec3();
			value.x = arrayValue[0].GetNumberValue();
			value.y = arrayValue[1].GetNumberValue();
			value.z = arrayValue[2].GetNumberValue();

			field->SetFieldValue<Vec2>(instance, value);
		}
		else if (fieldTypeId == TYPEID(Vec4))
		{
			if (!parent.IsObjectValue() && !parent.IsArrayValue())
				return;

			JArray& arrayValue = parent.IsObjectValue() ? parent.GetObjectValue()[fieldName].GetArrayValue() : parent.GetArrayValue()[index].GetArrayValue();

			Vec4 value = Vec4();
			value.x = arrayValue[0].GetNumberValue();
			value.y = arrayValue[1].GetNumberValue();
			value.z = arrayValue[2].GetNumberValue();
			value.w = arrayValue[3].GetNumberValue();

			field->SetFieldValue<Vec4>(instance, value);
		}
		else if (fieldTypeId == TYPEID(Color))
		{
			if (!parent.IsObjectValue() && !parent.IsArrayValue())
				return;

			JArray& arrayValue = parent.IsObjectValue() ? parent.GetObjectValue()[fieldName].GetArrayValue() : parent.GetArrayValue()[index].GetArrayValue();

			Color value = Color();
			value.r = arrayValue[0].GetNumberValue();
			value.g = arrayValue[1].GetNumberValue();
			value.b = arrayValue[2].GetNumberValue();
			value.a = arrayValue[3].GetNumberValue();

			field->SetFieldValue<Color>(instance, value);
		}
		else if (field->IsArrayField() && field->GetUnderlyingType() != nullptr && (field->GetUnderlyingType()->IsStruct() || field->GetUnderlyingType()->IsPOD()))
		{
			if (!parent.IsObjectValue() && !parent.IsArrayValue())
				return;

			JValue& arrayValue = parent.IsObjectValue() ? parent.GetObjectValue()[fieldName] : parent.GetArrayValue()[index];
			if (!arrayValue.IsArrayValue())
				return;

			field->ResizeArray(instance, arrayValue.GetSize());

			Array<u8>& array = const_cast<Array<u8>&>(field->GetFieldValue<Array<u8>>(instance));

			Array<Ptr<FieldType>> fieldList = field->GetArrayFieldListPtr(instance);

			if (!fieldList.IsEmpty())
			{
				void* arrayInstance = &array[0];

				for (int i = 0; i < fieldList.GetSize(); ++i)
				{
					DeserializeField(fieldList[i], arrayInstance, arrayValue, i);
				}
			}
		}
		else if (field->IsStructField() && field->GetDeclarationType() != nullptr)
		{
			if (!parent.IsObjectValue() && !parent.IsArrayValue())
				return;

			JValue& objectParent = parent.IsObjectValue() ? parent[fieldName] : parent.GetArrayValue()[index];
			if (!objectParent.IsObjectValue())
				return;

			void* structInstance = field->GetFieldInstance(instance);
			StructType* structType = (StructType*)field->GetDeclarationType();

			for (Ptr<FieldType> structField = structType->GetFirstField(); structField != nullptr; structField = structField->GetNext())
			{
				DeserializeField(structField, structInstance, objectParent, 0);
			}
		}
	}

	void Prefs::OnClassRegistered(ClassType* clazz)
	{
		if (clazz && clazz->HasAttribute("Prefs"))
		{
			Name prefsName = clazz->GetAttribute("Prefs").GetStringValue();
			if (prefsName.IsValid())
			{
				prefsClasses[prefsName].Add(clazz);
			}
		}
	}

	void Prefs::OnClassDeregistered(ClassType* clazz)
	{
		if (clazz && clazz->HasAttribute("Prefs"))
		{
			Name prefsName = clazz->GetAttribute("Prefs").GetStringValue();
			if (prefsName.IsValid())
			{
				prefsClasses[prefsName].Add(clazz);
			}
		}
	}


} // namespace CE
