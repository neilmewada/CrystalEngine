#include "CoreMinimal.h"

namespace CE
{

    JsonFieldSerializer::JsonFieldSerializer(StructType* instanceType, void* instance)
        : rawInstance(instance), writer(PrettyJsonWriter::Create(nullptr)), structType(instanceType)
    {
        fields.Clear();

        isMap = true;
            
        auto type = (StructType*)instanceType;
            
        for (auto field = type->GetFirstField(); field != nullptr; field = field->GetNext())
        {
            fields.Add(field);
        }
    }

    JsonFieldSerializer::JsonFieldSerializer(Array<Ptr<FieldType>> fields, void* instance)
        : rawInstance(instance), fields(fields), writer(PrettyJsonWriter::Create(nullptr))
    {
        isArray = true;
    }
    
    bool JsonFieldSerializer::HasNext()
    {
        return fields.NotEmpty() && IsValid();
    }

	int JsonFieldSerializer::Serialize(Stream* stream)
	{
		if (structType)
			structType->OnBeforeSerialize(rawInstance);

		while (HasNext())
		{
			WriteNext(stream);
		}

		if (stream->IsBinaryMode())
		{
			stream->Write('\0');
		}

		return 0;
	}

    bool JsonFieldSerializer::WriteNext(Stream* stream)
    {
        if (!IsValid() || !HasNext())
            return false;
        
        FieldType* field = GetNext();
		fields.RemoveAt(0);
        
        if (isFirstWrite)
        {
			if (writer.GetStream() == nullptr)
				writer = PrettyJsonWriter::Create(stream);

            if (isMap)
            {
                writer.WriteObjectStart();
            }
            else if (isArray)
            {
                writer.WriteArrayStart();
            }
        }
        
        isFirstWrite = false;
        
        if (field->GetDeclarationType() == nullptr || !field->IsSerialized())
        {
            return false;
        }
        
		TypeInfo* fieldDeclType = field->GetDeclarationType();
        TypeId fieldTypeId = fieldDeclType->GetTypeId();

		bool isVectorField = false;
		if (fieldTypeId == TYPEID(Vec2) || fieldTypeId == TYPEID(Vec3) || fieldTypeId == TYPEID(Vec4) ||
			fieldTypeId == TYPEID(Vec2i) || fieldTypeId == TYPEID(Vec3i) || fieldTypeId == TYPEID(Vec4i) || 
			fieldTypeId == TYPEID(Color))
		{
			isVectorField = true;
		}
        
        if (field->IsIntegerField())
        {
            if (isMap)
                writer.WriteIdentifier(field->GetName().GetString());
            
            if (fieldTypeId == TYPEID(u8))
                writer.WriteValue(field->GetFieldValue<u8>(rawInstance));
            else if (fieldTypeId == TYPEID(u16))
                writer.WriteValue(field->GetFieldValue<u16>(rawInstance));
            else if (fieldTypeId == TYPEID(u32))
                writer.WriteValue(field->GetFieldValue<u32>(rawInstance));
            else if (fieldTypeId == TYPEID(u64))
                writer.WriteValue(field->GetFieldValue<u64>(rawInstance));
            else if (fieldTypeId == TYPEID(s8))
                writer.WriteValue(field->GetFieldValue<s8>(rawInstance));
            else if (fieldTypeId == TYPEID(s16))
                writer.WriteValue(field->GetFieldValue<s16>(rawInstance));
            else if (fieldTypeId == TYPEID(s32))
                writer.WriteValue(field->GetFieldValue<s32>(rawInstance));
            else if (fieldTypeId == TYPEID(s64))
                writer.WriteValue(field->GetFieldValue<s64>(rawInstance));
        }
        else if (field->IsDecimalField())
        {
            if (isMap)
                writer.WriteIdentifier(field->GetName().GetString());
            
            if (fieldTypeId == TYPEID(f32))
                writer.WriteValue(field->GetFieldValue<f32>(rawInstance));
            else if (fieldTypeId == TYPEID(f64))
                writer.WriteValue((f32)field->GetFieldValue<f64>(rawInstance));
        }
		else if (field->IsEnumField()) // Serialize Enum as number
		{
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			writer.WriteValue(field->GetFieldEnumValue(rawInstance));
		}
		else if (isVectorField) // Serialize vector as array of numbers
		{
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			writer.WriteArrayStart();
			{
				if (fieldTypeId == TYPEID(Vec2))
				{
					Vec2 vec = field->GetFieldValue<Vec2>(rawInstance);
					writer.WriteValue(vec.x);
					writer.WriteValue(vec.y);
				}
				else if (fieldTypeId == TYPEID(Vec2i))
				{
					Vec2i vec = field->GetFieldValue<Vec2i>(rawInstance);
					writer.WriteValue(vec.x);
					writer.WriteValue(vec.y);
				}
				else if (fieldTypeId == TYPEID(Vec3))
				{
					Vec3 vec = field->GetFieldValue<Vec3>(rawInstance);
					writer.WriteValue(vec.x);
					writer.WriteValue(vec.y);
					writer.WriteValue(vec.z);
				}
				else if (fieldTypeId == TYPEID(Vec3i))
				{
					Vec3i vec = field->GetFieldValue<Vec3i>(rawInstance);
					writer.WriteValue(vec.x);
					writer.WriteValue(vec.y);
					writer.WriteValue(vec.z);
				}
				else if (fieldTypeId == TYPEID(Vec4))
				{
					Vec4 vec = field->GetFieldValue<Vec4>(rawInstance);
					writer.WriteValue(vec.x);
					writer.WriteValue(vec.y);
					writer.WriteValue(vec.z);
					writer.WriteValue(vec.w);
				}
				else if (fieldTypeId == TYPEID(Vec4i))
				{
					Vec4i vec = field->GetFieldValue<Vec4i>(rawInstance);
					writer.WriteValue(vec.x);
					writer.WriteValue(vec.y);
					writer.WriteValue(vec.z);
					writer.WriteValue(vec.w);
				}
				else if (fieldTypeId == TYPEID(Color))
				{
					Color color = field->GetFieldValue<Color>(rawInstance);
					writer.WriteValue(color.r);
					writer.WriteValue(color.g);
					writer.WriteValue(color.b);
					writer.WriteValue(color.a);
				}
			}
			writer.WriteArrayClose();
		}
        else if (fieldTypeId == TYPEID(Uuid))
        {
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			writer.WriteValue(field->GetFieldValue<Uuid>(rawInstance));
        }
        else if (fieldTypeId == TYPEID(String))
        {
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			const String& string = field->GetFieldValue<String>(rawInstance);
			writer.WriteValue(string);
        }
		else if (fieldTypeId == TYPEID(Name))
		{
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			writer.WriteValue(field->GetFieldValue<Name>(rawInstance).GetString());
		}
		else if (fieldTypeId == TYPEID(ClassType))
		{
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			ClassType* classType = field->GetFieldValue<ClassType*>(rawInstance);

			if (classType == nullptr)
			{
				writer.WriteNull();
			}
			else
			{
				writer.WriteValue(classType->GetTypeName().GetString());
			}
		}
		else if (fieldTypeId == TYPEID(StructType))
		{
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			StructType* structType = field->GetFieldValue<StructType*>(rawInstance);

			if (structType == nullptr)
			{
				writer.WriteNull();
			}
			else
			{
				writer.WriteValue(structType->GetTypeName().GetString());
			}
		}
		else if (fieldTypeId == TYPEID(EnumType))
		{
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			EnumType* enumType = field->GetFieldValue<EnumType*>(rawInstance);

			if (enumType == nullptr)
			{
				writer.WriteNull();
			}
			else
			{
				writer.WriteValue(enumType->GetTypeName().GetString());
			}
		}
		else if (fieldTypeId == TYPEID(SubClassType<>))
		{
			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			ClassType* classType = field->GetFieldValue<SubClassType<>>(rawInstance);

			if (classType == nullptr)
			{
				writer.WriteNull();
			}
			else
			{
				writer.WriteValue(classType->GetTypeName().GetString());
			}
		}
		else if (fieldDeclType->IsStruct())
		{
			StructType* structType = (StructType*)fieldDeclType;

			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			JsonFieldSerializer serializer{ structType, field->GetFieldInstance(rawInstance) };
			serializer.writer = this->writer;
			
			if (structType)
				structType->OnBeforeSerialize(rawInstance);

			if (serializer.HasNext())
			{
				while (serializer.HasNext())
				{
					serializer.WriteNext(stream);
				}
			}
			else
			{
				writer.WriteObjectStart();
				writer.WriteObjectClose();
			}

			this->writer = serializer.writer;
		}
		else if (field->IsArrayField())
		{
			Array<Ptr<FieldType>> fieldListPtr = field->GetArrayFieldListPtr(rawInstance);

			Array<u8> array = field->GetFieldValue<Array<u8>>(rawInstance);

			if (array.IsEmpty())
			{
				if (isMap)
					writer.WriteIdentifier(field->GetName().GetString());

				writer.WriteArrayStart();
				writer.WriteArrayClose();
			}
			else
			{
				if (isMap)
					writer.WriteIdentifier(field->GetName().GetString());

				JsonFieldSerializer serializer{ fieldListPtr, &array[0] };
				serializer.writer = this->writer;

				while (serializer.HasNext())
				{
					serializer.WriteNext(stream);
				}

				this->writer = serializer.writer;
			}
		}
		else if (field->IsObjectField())
		{
			Ref<Object> object;

			if (field->IsStrongRefCounted())
			{
				object = field->GetFieldValue<Ref<Object>>(rawInstance);
			}
			else if (field->IsWeakRefCounted())
			{
				object = field->GetFieldValue<WeakRef<Object>>(rawInstance).Lock();
			}
			else
			{
				object = field->GetFieldValue<Object*>(rawInstance);
			}

			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			if (object.IsNull())
			{
				writer.WriteNull();
			}
			else
			{
				Ref<Bundle> bundle = object->GetBundle();
				Uuid bundleUuid = {};
				if (bundle.IsValid())
				{
					bundleUuid = bundle->GetUuid();
				}

				writer.WriteObjectStart();
				{
					writer.WriteIdentifier("bundle");
					writer.WriteValue(bundleUuid);

					writer.WriteIdentifier("object");
					writer.WriteValue(object->GetUuid());
				}
				writer.WriteObjectClose();
			}
		}
		else if (field->IsEventField())
		{
			IScriptEvent* event = field->GetFieldEventValue(rawInstance);
			Array<FunctionBinding> bindings = event->GetSerializableBindings();

			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			writer.WriteArrayStart();

			for (const auto& binding : bindings)
			{
				Ref<Object> boundObject = binding.object.Lock();

				if (boundObject.IsNull())
				{
					writer.WriteNull();
					continue;
				}

				Ref<Bundle> bundle = boundObject->GetBundle();

				if (bundle.IsNull())
				{
					writer.WriteNull();
					continue;
				}

				writer.WriteObjectStart();
				{
					writer.WriteIdentifier("bundle");
					writer.WriteValue(bundle->GetUuid());

					writer.WriteIdentifier("object");
					writer.WriteValue(boundObject->GetUuid());

					writer.WriteIdentifier("function");
					writer.WriteValue(binding.function->GetName().GetString());
				}
				writer.WriteObjectClose();
			}

			writer.WriteArrayClose();
		}
		else if (field->IsDelegateField())
		{
			IScriptDelegate* delegate = field->GetFieldDelegateValue(rawInstance);

			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			FunctionBinding binding = delegate->GetBinding();

			if (isMap)
				writer.WriteIdentifier(field->GetName().GetString());

			Ref<Object> boundObject = binding.object.Lock();

			if (boundObject.IsValid() && boundObject->GetBundle() != nullptr)
			{
				writer.WriteObjectStart();
				{
					writer.WriteIdentifier("bundle");
					writer.WriteValue(boundObject->GetBundle()->GetUuid());

					writer.WriteIdentifier("object");
					writer.WriteValue(boundObject->GetUuid());

					writer.WriteIdentifier("function");
					writer.WriteValue(binding.function->GetName().GetString());
				}
				writer.WriteObjectClose();
			}
			else
			{
				writer.WriteNull();
			}
		}
        
        if (fields.IsEmpty())
        {
            if (isMap)
            {
                writer.WriteObjectClose();
            }
            else if (isArray)
            {
                writer.WriteArrayClose();
            }
        }
        
        return true;
    }

	bool JsonFieldSerializer::WriteNext(JsonValue* parentJson)
	{
		FieldType* field = GetNext();
		curIndex++;

		if (!IsValid() || !HasNext() || parentJson == nullptr)
			return false;
		if (!parentJson->IsObjectValue() && !parentJson->IsArrayValue())
			return false;
		if (isMap && !parentJson->IsObjectValue())
			return false;
		if (isArray && !parentJson->IsArrayValue())
			return false;

		if (field->GetDeclarationType() == nullptr || !field->IsSerialized())
		{
			return false;
		}

		fields.RemoveAt(0);

		TypeInfo* fieldDeclType = field->GetDeclarationType();
		TypeId fieldTypeId = fieldDeclType->GetTypeId();
		String fieldName = field->GetName().GetString();

		if (field->IsIntegerField())
		{
			JsonValue* json = new JsonValue((f64)0);
			if (isMap)
			{
				parentJson->GetObjectValue().Add({ fieldName, json });
			}
			else if (isArray)
			{
				parentJson->GetArrayValue().Add(json);
			}

			if (fieldTypeId == TYPEID(u8))
				*json = JsonValue((f64)field->GetFieldValue<u8>(rawInstance));
			else if (fieldTypeId == TYPEID(u16))
				*json = JsonValue((f64)field->GetFieldValue<u16>(rawInstance));
			else if (fieldTypeId == TYPEID(u32))
				*json = JsonValue((f64)field->GetFieldValue<u32>(rawInstance));
			else if (fieldTypeId == TYPEID(u64))
				*json = JsonValue((f64)field->GetFieldValue<u64>(rawInstance));
			else if (fieldTypeId == TYPEID(s8))
				*json = JsonValue((f64)field->GetFieldValue<s8>(rawInstance));
			else if (fieldTypeId == TYPEID(s16))
				*json = JsonValue((f64)field->GetFieldValue<s16>(rawInstance));
			else if (fieldTypeId == TYPEID(s32))
				*json = JsonValue((f64)field->GetFieldValue<s32>(rawInstance));
			else if (fieldTypeId == TYPEID(s64))
				*json = JsonValue((f64)field->GetFieldValue<s64>(rawInstance));
		}
		else if (field->IsDecimalField())
		{
			JsonValue* json = new JsonValue((f64)0);
			if (isMap)
			{
				parentJson->GetObjectValue().Add({ fieldName, json });
			}
			else if (isArray)
			{
				parentJson->GetArrayValue().Add(json);
			}

			if (fieldTypeId == TYPEID(f32))
				*json = JsonValue((f64)field->GetFieldValue<f32>(rawInstance));
			else if (fieldTypeId == TYPEID(f64))
				*json = JsonValue((f64)field->GetFieldValue<f64>(rawInstance));
		}
		else if (fieldTypeId == TYPEID(Uuid))
		{
			JsonValue* json = new JsonValue((f64)0);
			if (isMap)
			{
				parentJson->GetObjectValue().Add({ fieldName, json });
			}
			else if (isArray)
			{
				parentJson->GetArrayValue().Add(json);
			}

			*json = JsonValue(field->GetFieldValue<Uuid>(rawInstance).ToString());
		}
		else if (fieldTypeId == TYPEID(bool))
		{
			JsonValue* json = new JsonValue(false);
			if (isMap)
			{
				parentJson->GetObjectValue().Add({ fieldName, json });
			}
			else if (isArray)
			{
				parentJson->GetArrayValue().Add(json);
			}

			*json = JsonValue(field->GetFieldValue<bool>(rawInstance));
		}
		else if (fieldTypeId == TYPEID(String))
		{
			JsonValue* json = new JsonValue(String());
			if (isMap)
			{
				parentJson->GetObjectValue().Add({ fieldName, json });
			}
			else if (isArray)
			{
				parentJson->GetArrayValue().Add(json);
			}

			const String& string = field->GetFieldValue<String>(rawInstance);
			*json = JsonValue(string);
		}
		else if (fieldTypeId == TYPEID(Name))
		{
			JsonValue* json = new JsonValue(String());
			if (isMap)
			{
				parentJson->GetObjectValue().Add({ fieldName, json });
			}
			else if (isArray)
			{
				parentJson->GetArrayValue().Add(json);
			}

			*json = JsonValue(field->GetFieldValue<Name>(rawInstance).GetString());
		}
		else if (fieldDeclType->IsStruct())
		{
			JsonValue* json = new JsonValue(JsonObject());
			if (isMap)
			{
				parentJson->GetObjectValue().Add({ fieldName, json });
			}
			else if (isArray)
			{
				parentJson->GetArrayValue().Add(json);
			}

			StructType* structType = (StructType*)fieldDeclType;

			JsonFieldSerializer serializer{ structType, field->GetFieldInstance(rawInstance) };

			while (serializer.HasNext())
			{
				serializer.WriteNext(json);
			}
		}
		else if (field->IsArrayField())
		{
			JsonValue* json = new JsonValue(JsonArray());
			if (isMap)
			{
				parentJson->GetObjectValue().Add({ fieldName, json });
			}
			else if (isArray)
			{
				parentJson->GetArrayValue().Add(json);
			}

			Array<Ptr<FieldType>> fieldListPtr = field->GetArrayFieldListPtr(rawInstance);

			Array<u8> array = field->GetFieldValue<Array<u8>>(rawInstance);

			if (!fieldListPtr.IsEmpty())
			{
				JsonFieldSerializer serializer = JsonFieldSerializer(fieldListPtr, &array[0]);

				while (serializer.HasNext())
				{
					serializer.WriteNext(json);
				}
			}
		}

		if (fields.IsEmpty())
		{
			if (isMap)
			{
				writer.WriteObjectClose();
			}
			else if (isArray)
			{
				writer.WriteArrayClose();
			}
		}

		return true;
	}

	bool JsonFieldSerializer::WriteNext(JValue& parentJson)
	{
		FieldType* field = GetNext();
		curIndex++;

		if (!IsValid() || !HasNext() || parentJson.IsNullValue())
			return false;

		fields.RemoveAt(0);

		if (!parentJson.IsObjectValue() && !parentJson.IsArrayValue())
			return false;
		if (isMap && !parentJson.IsObjectValue())
			return false;
		if (isArray && !parentJson.IsArrayValue())
			return false;
		if (!isMap && !isArray)
			return false;

		if (field->GetDeclarationType() == nullptr || !field->IsSerialized())
		{
			return false;
		}

		TypeInfo* fieldDeclType = field->GetDeclarationType();
		TypeId fieldTypeId = fieldDeclType->GetTypeId();
		String fieldName = field->GetName().GetString();

		if (field->IsIntegerField())
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue(0) });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(0));
				json = &parentJson.GetArrayValue().Top();
			}

			if (fieldTypeId == TYPEID(u8))
				*json = JValue((f64)field->GetFieldValue<u8>(rawInstance));
			else if (fieldTypeId == TYPEID(u16))
				*json = JValue((f64)field->GetFieldValue<u16>(rawInstance));
			else if (fieldTypeId == TYPEID(u32))
				*json = JValue((f64)field->GetFieldValue<u32>(rawInstance));
			else if (fieldTypeId == TYPEID(u64))
				*json = JValue((f64)field->GetFieldValue<u64>(rawInstance));
			else if (fieldTypeId == TYPEID(s8))
				*json = JValue((f64)field->GetFieldValue<s8>(rawInstance));
			else if (fieldTypeId == TYPEID(s16))
				*json = JValue((f64)field->GetFieldValue<s16>(rawInstance));
			else if (fieldTypeId == TYPEID(s32))
				*json = JValue((f64)field->GetFieldValue<s32>(rawInstance));
			else if (fieldTypeId == TYPEID(s64))
				*json = JValue((f64)field->GetFieldValue<s64>(rawInstance));
		}
		else if (field->IsDecimalField())
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue(0) });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(0));
				json = &parentJson.GetArrayValue().Top();
			}

			if (fieldTypeId == TYPEID(f32))
				*json = JValue((f64)field->GetFieldValue<f32>(rawInstance));
			else if (fieldTypeId == TYPEID(f64))
				*json = JValue((f64)field->GetFieldValue<f64>(rawInstance));
		}
		else if (fieldTypeId == TYPEID(Uuid))
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue(0) });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(0));
				json = &parentJson.GetArrayValue().Top();
			}

			*json = JValue(field->GetFieldValue<Uuid>(rawInstance).ToString());
		}
		else if (fieldTypeId == TYPEID(bool))
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue(false) });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(false));
				json = &parentJson.GetArrayValue().Top();
			}

			*json = JValue(field->GetFieldValue<bool>(rawInstance));
		}
		else if (fieldTypeId == TYPEID(String))
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue("") });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(""));
				json = &parentJson.GetArrayValue().Top();
			}

			const String& string = field->GetFieldValue<String>(rawInstance);
			*json = JValue(string);
		}
		else if (fieldTypeId == TYPEID(Name))
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue("") });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(""));
				json = &parentJson.GetArrayValue().Top();
			}

			*json = JValue(field->GetFieldValue<Name>(rawInstance).GetString());
		}
		else if (fieldTypeId == TYPEID(SubClassType<Object>))
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue("") });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(""));
				json = &parentJson.GetArrayValue().Top();
			}

			ClassType* classType = field->GetFieldValue<SubClassType<Object>>(rawInstance);

			if (classType != nullptr)
			{
				*json = JValue(classType->GetTypeName().GetString());
			}
			else
			{
				*json = JValue(String());
			}
		}
		else if (field->IsEnumField()) // Serialize enum as an integer
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue(0) });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(0));
				json = &parentJson.GetArrayValue().Top();
			}

			*json = JValue(field->GetFieldEnumValue(rawInstance));
		}
		else if (fieldDeclType->IsStruct())
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue(JObject()) });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(JObject()));
				json = &parentJson.GetArrayValue().Top();
			}

			StructType* structType = (StructType*)fieldDeclType;

			JsonFieldSerializer serializer{ structType, field->GetFieldInstance(rawInstance) };

			structType->OnBeforeSerialize(field->GetFieldInstance(rawInstance));

			while (serializer.HasNext())
			{
				serializer.WriteNext(*json);
			}
		}
		else if (field->IsArrayField())
		{
			JValue* json = nullptr;
			if (isMap)
			{
				parentJson.GetObjectValue().Add({ fieldName, JValue(JArray()) });
				json = &parentJson.GetObjectValue()[fieldName];
			}
			else if (isArray)
			{
				parentJson.GetArrayValue().Add(JValue(JArray()));
				json = &parentJson.GetArrayValue().Top();
			}

			Array<Ptr<FieldType>> fieldListPtr = field->GetArrayFieldListPtr(rawInstance);

			Array<u8> array = field->GetFieldValue<Array<u8>>(rawInstance);

			if (!fieldListPtr.IsEmpty())
			{
				JsonFieldSerializer serializer = JsonFieldSerializer(fieldListPtr, &array[0]);

				while (serializer.HasNext())
				{
					serializer.WriteNext(*json);
				}
			}
		}

		if (fields.IsEmpty())
		{
			if (isMap)
			{
				writer.WriteObjectClose();
			}
			else if (isArray)
			{
				writer.WriteArrayClose();
			}
		}

		return true;
	}

	Ptr<FieldType> JsonFieldSerializer::GetNext()
	{
        if (!HasNext())
            return nullptr;

        return fields[0];
    }
    
    
	JsonFieldDeserializer::JsonFieldDeserializer(StructType* instanceType, void* instance)
		: rawInstance(instance)
		, structType(instanceType)
	{
		fields.Clear();

		isMap = true;

		auto type = (StructType*)instanceType;

		for (auto field = type->GetFirstField(); field != nullptr; field = field->GetNext())
		{
			fields.Add(field);
		}
	}

	JsonFieldDeserializer::JsonFieldDeserializer(Array<Ptr<FieldType>> fields, void* instance)
		: rawInstance(instance), fields(fields)
	{
		isArray = true;
	}

	JsonFieldDeserializer::~JsonFieldDeserializer()
	{
		delete rootJson;
		rootJson = nullptr;
	}

	bool JsonFieldDeserializer::HasNext()
	{
		return fields.NotEmpty();
	}

	int JsonFieldDeserializer::Deserialize(Stream* stream)
	{
		JValue root{};
		JsonSerializer::Deserialize2(stream, root);

		while (HasNext())
		{
			ReadNext(root);
		}

		if (structType != nullptr)
		{
			structType->OnAfterDeserialize(rawInstance);
		}

		return 0;
	}

	bool JsonFieldDeserializer::ReadNext(Stream* stream)
	{
		if (!IsValid() || !HasNext())
			return false;

		if (isFirstRead && rootJson == nullptr)
		{
			rootJson = JsonSerializer::Deserialize(stream);
		}

		isFirstRead = false;

		if (rootJson == nullptr)
			return false;

		if (rootJson->IsObjectValue() && isMap)
		{
			JsonObject& objectValue = rootJson->GetObjectValue();
			for (const auto& [key, value] : objectValue)
			{
                const String& keyString = key;
				s32 idx = fields.IndexOf([=](FieldType* fieldType) {
                    return fieldType->GetName().GetString() == keyString; });
				if (idx < 0)
					continue;

				FieldType* field = fields[idx];
				fields.RemoveAt(idx);

				auto result = ReadField(field, value);

				delete value;
				objectValue.Remove(key);
				return result;
			}
		}
		else if (rootJson->IsArrayValue() && isArray)
		{
			JsonArray& arrayValue = rootJson->GetArrayValue();

			if (arrayValue.IsEmpty() || fields.IsEmpty())
			{
				return false;
			}

			FieldType* field = fields[0];
			fields.RemoveAt(0);
			
			auto result = ReadField(field, arrayValue[0]);

			delete arrayValue[0];
			arrayValue.RemoveAt(0);
			return result;
		}
		else
		{
			return false;
		}

		return true;
	}

	bool JsonFieldDeserializer::ReadNext(const JValue& parentJson)
	{
		if (fields.IsEmpty())
			return false;

		auto field = fields[0];
		fields.RemoveAt(0);
		auto fieldName = field->GetName().GetString();

		if (isArray && !parentJson.IndexExists(arrayIndex))
			return false;

		if (isMap)
		{
			if (parentJson.KeyExists(fieldName))
				; // Do nothing
			else if (parentJson.KeyExists(fieldName.ToPascalCase()))
				fieldName = fieldName.ToPascalCase();
			else
				return false;
		}

		const auto& json = isMap ? parentJson.GetObjectValue().Get(fieldName) : parentJson.GetArrayValue()[arrayIndex++];

		if (field == nullptr || json.IsNullValue() || field->GetDeclarationType() == nullptr)
			return false;

		auto fieldDeclType = field->GetDeclarationType();
		auto fieldTypeId = field->GetDeclarationTypeId();
		auto underlyingType = field->GetUnderlyingType();

		bool isVectorField = false;
		if (fieldTypeId == TYPEID(Vec2) || fieldTypeId == TYPEID(Vec3) || fieldTypeId == TYPEID(Vec4) || fieldTypeId == TYPEID(Rect) ||
			fieldTypeId == TYPEID(Vec2i) || fieldTypeId == TYPEID(Vec3i) || fieldTypeId == TYPEID(Vec4i) ||
			fieldTypeId == TYPEID(Color))
		{
			isVectorField = true;
		}
		
		if (json.IsNumberValue() && (field->IsIntegerField() || field->IsDecimalField() || field->IsEnumField()))
		{
			if (fieldTypeId == TYPEID(s8))
				field->SetFieldValue<s8>(rawInstance, (s8)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(s16))
				field->SetFieldValue<s16>(rawInstance, (s16)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(s32))
				field->SetFieldValue<s32>(rawInstance, (s32)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(s64))
				field->SetFieldValue<s64>(rawInstance, (s64)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(u8))
				field->SetFieldValue<u8>(rawInstance, (u8)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(u16))
				field->SetFieldValue<u16>(rawInstance, (u16)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(u32))
				field->SetFieldValue<u32>(rawInstance, (u32)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(u64))
				field->SetFieldValue<u64>(rawInstance, (u64)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(f32))
				field->SetFieldValue<f32>(rawInstance, (f32)json.GetNumberValue());
			else if (fieldTypeId == TYPEID(f64))
				field->SetFieldValue<f64>(rawInstance, (f64)json.GetNumberValue());
			else if (field->IsEnumField()) // Deserialize enum from number
				field->SetFieldEnumValue(rawInstance, (s64)json.GetNumberValue());
			else
				return false;

			return true;
		}
		else if (json.IsBoolValue())
		{
			if (fieldTypeId == TYPEID(bool))
				field->SetFieldValue<bool>(rawInstance, (bool)json.GetBoolValue());
			else if (fieldTypeId == TYPEID(b8))
				field->SetFieldValue<b8>(rawInstance, (b8)json.GetBoolValue());
			else
				return false;

			return true;
		}
		else if (json.IsStringValue())
		{
			if (fieldTypeId == TYPEID(String))
				field->SetFieldValue<String>(rawInstance, json.GetStringValue());
			else if (fieldTypeId == TYPEID(Name))
				field->SetFieldValue<Name>(rawInstance, json.GetStringValue());
			else if (fieldTypeId == TYPEID(IO::Path))
				field->SetFieldValue<IO::Path>(rawInstance, IO::Path(json.GetStringValue()));
			else if (fieldTypeId == TYPEID(Uuid))
				field->SetFieldValue<Uuid>(rawInstance, Uuid::FromString(json.GetStringValue()));
			else if (fieldTypeId == TYPEID(SubClassType<Object>) && underlyingType != nullptr)
			{
				SubClassType<Object>& value = const_cast<SubClassType<Object>&>(field->GetFieldValue<SubClassType<Object>>(rawInstance));
				ClassType* baseClassType = (ClassType*)underlyingType;
				ClassType* classType = ClassType::FindClass(json.GetStringValue());
				if (classType != nullptr && classType->IsSubclassOf(baseClassType))
				{
					value = classType;
				}
				else
				{
					value = nullptr;
				}
			}
			else if (fieldTypeId == TYPEID(ClassType))
			{
				ClassType* classType = ClassType::FindClass(json.GetStringValue());

				field->SetFieldValue<ClassType*>(rawInstance, classType);
			}
			else if (fieldTypeId == TYPEID(StructType))
			{
				StructType* structType = StructType::FindStruct(json.GetStringValue());

				field->SetFieldValue<StructType*>(rawInstance, structType);
			}
			else if (fieldTypeId == TYPEID(EnumType))
			{
				EnumType* enumType = EnumType::FindEnum(json.GetStringValue());

				field->SetFieldValue<EnumType*>(rawInstance, enumType);
			}
			else if (field->IsEnumField()) // Deserialize enum from string
			{
				EnumType* enumType = (EnumType*)fieldDeclType;
				EnumConstant* enumConstant = enumType->FindConstantWithName(json.GetStringValue());
				if (enumConstant != nullptr)
				{
					field->SetFieldEnumValue(rawInstance, enumConstant->GetValue());
				}
			}
			else
			{
				return false;
			}

			return true;
		}
		else if (json.IsStringValue() && fieldTypeId == TYPEID(Uuid))
		{
			field->SetFieldValue<Uuid>(rawInstance, Uuid::FromString(json.GetStringValue()));

			return true;
		}
		else if (json.IsObjectValue() && fieldDeclType->IsStruct())
		{
			void* structInstance = field->GetFieldInstance(rawInstance);

			auto structType = (StructType*)fieldDeclType;
			structType->InitializeDefaults(structInstance);

			JsonFieldDeserializer deserializer{ structType, structInstance };
			deserializer.isFirstRead = false;
			deserializer.isMap = true;
			deserializer.isArray = false;

			while (deserializer.HasNext())
			{
				deserializer.ReadNext(json);
			}

			if (structType != nullptr)
			{
				structType->OnAfterDeserialize(structInstance);
			}

			return true;
		}
		else if (json.IsObjectValue() && fieldDeclType->IsObject() && 
			json.GetObjectValue().KeyExists("bundle") && 
			json.GetObjectValue().KeyExists("object"))
		{
			String bundleUuidStr = json.GetObjectValue().Get("bundle").GetStringValue();
			String objectUuidStr = json.GetObjectValue().Get("object").GetStringValue();

			Uuid bundleUuid = Uuid::FromString(bundleUuidStr);
			Uuid objectUuid = Uuid::FromString(objectUuidStr);

			Ref<Bundle> bundle = Bundle::LoadBundle(nullptr, bundleUuid, { .loadFully = false, .forceReload = false });
			Ref<Object> object = nullptr;

			if (bundle.IsValid())
			{
				object = bundle->LoadObject(objectUuid);
			}

			field->SetFieldObjectValue(rawInstance, object);

			return true;
		}
		else if (json.IsObjectValue() && (fieldTypeId == TYPEID(Vec4) || fieldTypeId == TYPEID(Vec4i)))
		{
			Vec4 value = {};
			const JObject& objectValue = json.GetObjectValue();
			if (objectValue.KeyExists("left"))
				value.left = objectValue.Get("left").GetNumberValue();
			else if (objectValue.KeyExists("Left"))
				value.left = objectValue.Get("Left").GetNumberValue();

			if (objectValue.KeyExists("right"))
				value.right = objectValue.Get("right").GetNumberValue();
			else if (objectValue.KeyExists("Right"))
				value.right = objectValue.Get("Right").GetNumberValue();

			if (objectValue.KeyExists("top"))
				value.top = objectValue.Get("top").GetNumberValue();
			else if (objectValue.KeyExists("Top"))
				value.top = objectValue.Get("Top").GetNumberValue();

			if (objectValue.KeyExists("bottom"))
				value.bottom = objectValue.Get("bottom").GetNumberValue();
			else if (objectValue.KeyExists("Bottom"))
				value.bottom = objectValue.Get("Bottom").GetNumberValue();

			if (fieldTypeId == TYPEID(Vec4))
			{
				Vec4& val = const_cast<Vec4&>(field->GetFieldValue<Vec4>(rawInstance));
				val = value;
			}
			else if (fieldTypeId == TYPEID(Vec4i))
			{
				Vec4i& val = const_cast<Vec4i&>(field->GetFieldValue<Vec4i>(rawInstance));
				val.left   = lround(value.left);
				val.right  = lround(value.right);
				val.top    = lround(value.top);
				val.bottom = lround(value.bottom);
			}

			return true;
		}
		else if (json.IsArrayValue() && fieldDeclType->IsArrayType())
		{
			if (json.GetArrayValue().IsEmpty())
			{
				field->ResizeArray(rawInstance, 0);
				return true;
			}

			field->ResizeArray(rawInstance, json.GetArrayValue().GetSize());
			Array<Ptr<FieldType>> fieldListPtr = field->GetArrayFieldListPtr(rawInstance);

			u8* arrayInstance = const_cast<u8*>(&field->GetFieldValue<Array<u8>>(rawInstance)[0]);

			JsonFieldDeserializer deserializer{ fieldListPtr, arrayInstance };
			deserializer.isMap = false;
			deserializer.isArray = true;

			while (deserializer.HasNext())
			{
				deserializer.ReadNext(json);
			}

			return true;
		}
		else if (json.IsArrayValue() && field->IsEnumFlagsField()) // Array of enum flags, i.e. array string values
		{
			if (json.GetArrayValue().IsEmpty())
			{
				field->SetFieldEnumValue(rawInstance, 0);
				return true;
			}

			EnumType* enumType = (EnumType*)fieldDeclType;

			const JArray& flagsArray = json.GetArrayValue();
			s64 enumFlags = 0;

			for (int i = 0; i < flagsArray.GetSize(); i++)
			{
				if (!flagsArray[i].IsStringValue())
					continue;
				
				String flagName = flagsArray[i].GetStringValue();
				EnumConstant* constant = enumType->FindConstantWithName(flagName);
				if (constant == nullptr)
					continue;

				enumFlags |= constant->GetValue();
			}

			field->SetFieldEnumValue(rawInstance, enumFlags);

			return true;
		}
		else if (json.IsArrayValue() && isVectorField) // Vector (float array)
		{
			const JArray& arrayValue = json.GetArrayValue();
			Vec4 vec4 = {};
			Vec4i vec4i = {};
			int idx = 0;
			
			for (int i = 0; i < arrayValue.GetSize() && idx < 4; i++)
			{
				if (!arrayValue[i].IsNumberValue())
					continue;

				vec4[idx] = (f32)arrayValue[i].GetNumberValue();
				vec4i[idx] = (s32)arrayValue[i].GetNumberValue();
				idx++;
			}

			if (fieldTypeId == TYPEID(Vec2))
				field->SetFieldValue<Vec2>(rawInstance, vec4);
			else if (fieldTypeId == TYPEID(Vec3))
				field->SetFieldValue<Vec3>(rawInstance, vec4);
			else if (fieldTypeId == TYPEID(Vec4))
				field->SetFieldValue<Vec4>(rawInstance, vec4);
			else if (fieldTypeId == TYPEID(Rect))
				field->SetFieldValue<Rect>(rawInstance, Rect(vec4.x, vec4.y, vec4.z, vec4.w));
			else if (fieldTypeId == TYPEID(Vec2i))
				field->SetFieldValue<Vec2i>(rawInstance, vec4i);
			else if (fieldTypeId == TYPEID(Vec3i))
				field->SetFieldValue<Vec3i>(rawInstance, vec4i);
			else if (fieldTypeId == TYPEID(Vec4i))
				field->SetFieldValue<Vec4i>(rawInstance, vec4i);
			else if (fieldTypeId == TYPEID(Color))
				field->SetFieldValue<Color>(rawInstance, Color(vec4.x, vec4.y, vec4.z, vec4.w));

			return true;
		}
		else if (json.IsArrayValue() && field->IsEventField())
		{
			IScriptEvent* event = field->GetFieldEventValue(rawInstance);
			event->UnbindAll();

			for (int i = 0; i < json.GetArrayValue().GetSize(); ++i)
			{
				const JValue& element = json.GetArrayValue()[i];
				if (!element.IsObjectValue())
					continue;

				const JObject& objectValue = element.GetObjectValue();
				if (!objectValue.KeyExists("object") || 
					!objectValue.KeyExists("bundle") ||
					!objectValue.KeyExists("function") ||
					!objectValue.Get("object").IsStringValue() ||
					!objectValue.Get("bundle").IsStringValue() ||
					!objectValue.Get("function").IsStringValue())
					continue;

				Uuid objectUuid = Uuid::FromString(objectValue.Get("object").GetStringValue());
				Uuid bundleUuid = Uuid::FromString(objectValue.Get("bundle").GetStringValue());
				String functionName = objectValue.Get("function").GetStringValue();

				Ref<Bundle> bundle = Bundle::LoadBundle(nullptr, bundleUuid, {
					.loadFully = true,
					.forceReload = false,
				});

				if (bundle.IsValid())
				{
					Ref<Object> objectRef = bundle->LoadObject(objectUuid);
					if (objectRef.IsValid())
					{
						FunctionType* function = objectRef->GetClass()->FindFunctionWithName(functionName);
						if (function != nullptr)
						{
							event->Bind(objectRef, function);
						}
					}
				}
			}

			return true;
		}
		else if (json.IsObjectValue() && field->IsDelegateField())
		{
			IScriptDelegate* delegate = field->GetFieldDelegateValue(rawInstance);
			delegate->Unbind();

			const JObject& objectValue = json.GetObjectValue();
			if (!objectValue.KeyExists("object") ||
				!objectValue.KeyExists("bundle") ||
				!objectValue.KeyExists("function") ||
				!objectValue.Get("object").IsStringValue() ||
				!objectValue.Get("bundle").IsStringValue() ||
				!objectValue.Get("function").IsStringValue())
			{
				return false;
			}

			Uuid objectUuid = Uuid::FromString(objectValue.Get("object").GetStringValue());
			Uuid bundleUuid = Uuid::FromString(objectValue.Get("bundle").GetStringValue());
			String functionName = objectValue.Get("function").GetStringValue();

			Ref<Bundle> bundle = Bundle::LoadBundle(nullptr, bundleUuid, {
				.loadFully = true,
				.forceReload = false,
			});

			if (bundle.IsValid())
			{
				Ref<Object> objectRef = bundle->LoadObject(objectUuid);
				if (objectRef.IsValid())
				{
					FunctionType* function = objectRef->GetClass()->FindFunctionWithName(functionName);
					if (function != nullptr)
					{
						delegate->Bind(objectRef, function);
					}
				}
			}

			return true;
		}
		else if (json.IsNullValue())
		{
			return true;
		}

		return false;
	}

	bool JsonFieldDeserializer::ReadNextField(JsonValue* jsonValue)
	{
		if (fields.IsEmpty())
			return false;

		auto field = fields[0];

		return ReadField(field, jsonValue);
	}

	bool JsonFieldDeserializer::ReadField(const Ptr<FieldType>& field, JsonValue* jsonValue)
	{
		if (field == nullptr || jsonValue == nullptr || field->GetDeclarationType() == nullptr)
			return false;

		auto fieldDeclType = field->GetDeclarationType();
		auto fieldTypeId = field->GetDeclarationTypeId();

		if (jsonValue->IsNumberValue() && (field->IsIntegerField() || field->IsDecimalField()))
		{
			if (fieldTypeId == TYPEID(s8))
				field->SetFieldValue<s8>(rawInstance, (s8)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(s16))
				field->SetFieldValue<s16>(rawInstance, (s16)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(s32))
				field->SetFieldValue<s32>(rawInstance, (s32)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(s64))
				field->SetFieldValue<s64>(rawInstance, (s64)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(u8))
				field->SetFieldValue<u8>(rawInstance, (u8)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(u16))
				field->SetFieldValue<u16>(rawInstance, (u16)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(u32))
				field->SetFieldValue<u32>(rawInstance, (u32)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(u64))
				field->SetFieldValue<u64>(rawInstance, (u64)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(f32))
				field->SetFieldValue<f32>(rawInstance, (f32)jsonValue->GetNumberValue());
			else if (fieldTypeId == TYPEID(f64))
				field->SetFieldValue<f64>(rawInstance, (f64)jsonValue->GetNumberValue());
			else
				return false;

			return true;
		}
		else if (jsonValue->IsBoolValue())
		{
			if (fieldTypeId == TYPEID(bool))
				field->SetFieldValue<bool>(rawInstance, (bool)jsonValue->GetBoolValue());
			else if (fieldTypeId == TYPEID(b8))
				field->SetFieldValue<b8>(rawInstance, (b8)jsonValue->GetBoolValue());
			else
				return false;

			return true;
		}
		else if (jsonValue->IsStringValue())
		{
			if (fieldTypeId == TYPEID(String))
				field->SetFieldValue<String>(rawInstance, jsonValue->GetStringValue());
			else if (fieldTypeId == TYPEID(Name))
				field->SetFieldValue<Name>(rawInstance, jsonValue->GetStringValue());
			else if (fieldTypeId == TYPEID(IO::Path))
				field->SetFieldValue<IO::Path>(rawInstance, IO::Path(jsonValue->GetStringValue()));
			else if (fieldTypeId == TYPEID(Uuid))
				field->SetFieldValue<Uuid>(rawInstance, Uuid::FromString(jsonValue->GetStringValue()));
			else
				return false;

			return true;
		}
		else if (jsonValue->IsObjectValue() && fieldDeclType->IsStruct())
		{
			void* structInstance = field->GetFieldInstance(rawInstance);

			auto structType = (StructType*)fieldDeclType;
			structType->InitializeDefaults(structInstance);

			JsonFieldDeserializer deserializer{ structType, structInstance };
			deserializer.isFirstRead = false;
			deserializer.rootJson = jsonValue;
			deserializer.isMap = true;
			deserializer.isArray = false;

			while (deserializer.HasNext())
			{
				deserializer.ReadNext(nullptr);
			}

			deserializer.rootJson = nullptr; // Unset json to prevent destruction

			return true;
		}
		else if (jsonValue->IsObjectValue() && fieldDeclType->IsClass())
		{
			Object* classInstance = field->GetFieldValue<Object*>(rawInstance);
			if (classInstance == nullptr)
				return true;

			JsonFieldDeserializer deserializer{ (StructType*)fieldDeclType, classInstance };
			deserializer.isFirstRead = false;
			deserializer.rootJson = jsonValue;
			deserializer.isMap = true;
			deserializer.isArray = false;

			while (deserializer.HasNext())
			{
				deserializer.ReadNext(nullptr);
			}

			deserializer.rootJson = nullptr; // Unset json to prevent destruction

			return true;
		}
		else if (jsonValue->IsArrayValue() && fieldDeclType->IsArrayType())
		{
			if (jsonValue->GetArrayValue().IsEmpty())
			{
				field->ResizeArray(rawInstance, 0);
				return true;
			}

			field->ResizeArray(rawInstance, jsonValue->GetArrayValue().GetSize());
			Array<Ptr<FieldType>> fieldListPtr = field->GetArrayFieldListPtr(rawInstance);

			u8* arrayInstance = const_cast<u8*>(&field->GetFieldValue<Array<u8>>(rawInstance)[0]);

			JsonFieldDeserializer deserializer{ fieldListPtr, arrayInstance };
			deserializer.isFirstRead = false;
			deserializer.rootJson = jsonValue;
			deserializer.isMap = false;
			deserializer.isArray = true;

			while (deserializer.HasNext())
			{
				deserializer.ReadNext(nullptr);
			}

			deserializer.rootJson = nullptr; // Unset json to prevent deletion

			return true;
		}
		else if (jsonValue->IsNullValue())
		{
			return true;
		}

		return false;
	}

} // namespace CE
