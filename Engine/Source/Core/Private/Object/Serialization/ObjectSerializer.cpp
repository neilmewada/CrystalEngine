
#include "Core.h"

#include "../Bundle.inl"
#include "Json/json.hpp"
#include "spdlog/fmt/bundled/compile.h"

namespace CE
{
    static HashMap<TypeId, u8> FieldTypeBytes{
        { 0, 0 },
        { TYPEID(u8), 1 },
		{ TYPEID(u16), 2 },
		{ TYPEID(u32), 3 },
		{ TYPEID(u64), 4 },
		{ TYPEID(s8), 5 },
		{ TYPEID(s16), 6 },
		{ TYPEID(s32), 7 },
		{ TYPEID(s64), 8 },
		{ TYPEID(f32), 9 },
		{ TYPEID(f64), 10 },
		{ TYPEID(b8), 11 },
        { TYPEID(String), 12 }, { TYPEID(Name), 12 }, { TYPEID(IO::Path), 12 },
        { TYPEID(Vec2), 13 },
        { TYPEID(Vec3), 14 },
        { TYPEID(Vec4), 15 },
        { TYPEID(Color), 15 },
        { TYPEID(Vec2i), 16 },
        { TYPEID(Vec3i), 17 },
        { TYPEID(Vec4i), 18 },
        { TYPEID(BinaryBlob), 0x16 },
        { TYPEID(Object), 0x17 },
        { TYPEID(ScriptDelegate<>), 0x18 },
        { TYPEID(ScriptEvent<>), 0x19 },
        { TYPEID(Uuid), 0x1B },
        { TYPEID(ObjectMap), 0x1C }
    };

    static constexpr u8 ObjectArrayFieldType = 0x13;
    static constexpr u8 StructArrayFieldType = 0x14;
    static constexpr u8 SimpleArrayFieldType = 0x15;
    static constexpr u8 StructFieldType = 0x1A;

    static HashSet<u8> Optional1Requirement = { StructFieldType, StructArrayFieldType };
    static HashSet<u8> Optional2Requirement = { SimpleArrayFieldType };

    static void SaveBundleDependencies(Stream* stream, const Array<Uuid>& bundleDependencies)
    {
        *stream << (u32)bundleDependencies.GetSize();

        for (int i = 0; i < bundleDependencies.GetSize(); ++i)
        {
            *stream << bundleDependencies[i];
        }
    }

    Ref<Bundle> Bundle::LoadBundle(const Ref<Object>& outer, Stream* stream, BundleLoadResult& outResult, const LoadBundleArgs& loadArgs)
    {
        ZoneScoped;

        if (stream == nullptr || !stream->CanRead())
        {
            CE_LOG(Error, All, "Failed to load bundle: Stream is either NULL or not readable!");
            outResult = BundleLoadResult::InvalidStream;
            return nullptr;
        }

        stream->SetBinaryMode(true);

        u64 magicNumber = 0;
        *stream >> magicNumber;

        if (magicNumber != BUNDLE_MAGIC_NUMBER)
        {
            outResult = BundleLoadResult::InvalidBundle;
            return nullptr;
        }

        u32 headerChecksum = 0;
        *stream >> headerChecksum;

        if (headerChecksum != 0)
        {
            outResult = BundleLoadResult::UnsupportedBundleVersion;
            return nullptr;
        }

        u32 majorVersion = 0;
        u32 minorVersion = 0;
        u32 patchVersion = 0;

        *stream >> majorVersion;
        *stream >> minorVersion;
        *stream >> patchVersion;

        if (majorVersion != BUNDLE_VERSION_MAJOR)
        {
            outResult = BundleLoadResult::UnsupportedBundleVersion;
            return nullptr;
        }

        u64 schemaTableStartOffset = 0;
        *stream >> schemaTableStartOffset;
        u64 serializedDataStartOffset = 0;
        *stream >> serializedDataStartOffset;

        Uuid bundleUuid = Uuid::Zero();
        *stream >> bundleUuid;
        String bundleName = "";
        *stream >> bundleName;

        u32 numBundleDependencies = 0;
        *stream >> numBundleDependencies;
        Array<Uuid> externalBundleUuids;

        for (int i = 0; i < numBundleDependencies; ++i)
        {
            Uuid uuid;
            *stream >> uuid;
            externalBundleUuids.Add(uuid);
        }

        u8 isCooked = 0;
        *stream >> isCooked;

        Ref<Bundle> bundle = nullptr;

        if (loadArgs.loadTemporary)
        {
            Internal::ObjectCreateParams params{ Bundle::Type() };
            params.name = bundleName;
            params.uuid = bundleUuid;
            params.templateObject = nullptr;
            params.objectFlags = OF_TemporaryBundleInstance;
            params.outer = outer.Get();

            bundle = (Bundle*)Internal::CreateObjectInternal(params);
            bundle->isFullyLoaded = false;
            bundle->isLoadedFromDisk = true;
        }
        else
        {
            LockGuard lock{ bundleRegistryMutex };

            if (loadedBundlesByUuid.KeyExists(bundleUuid)) // Bundle is already loaded
            {
                bundle = loadedBundlesByUuid[bundleUuid].Lock();
            }

            if (bundle.IsNull())
            {
                Internal::ObjectCreateParams params{ Bundle::Type() };
                params.name = bundleName;
                params.uuid = bundleUuid;
                params.templateObject = nullptr;
                params.objectFlags = OF_NoFlags;
                params.outer = outer.Get();

                bundle = (Bundle*)Internal::CreateObjectInternal(params);
                bundle->isFullyLoaded = false;
                bundle->isLoadedFromDisk = true;

                loadedBundlesByUuid[bundleUuid] = bundle;
            }
        }

        if (bundle->isFullyLoaded && !loadArgs.forceReload)
        {
            return bundle;
        }

        // Only 1 thread can load/store the same bundle at once
        LockGuard bundleLock{ bundle->bundleMutex };

        bundle->readerStream = stream;
        defer(=)
        {
            bundle->readerStream = nullptr;
        };

        {
            LockGuard loadedObjectsLock{ bundle->loadedObjectsMutex };

            //bundle->loadedObjectsByUuid.Clear();
	        bundle->loadedObjectsByUuid[bundleUuid] = bundle;
        }

    	bundle->majorVersion = majorVersion;
        bundle->minorVersion = minorVersion;

        bundle->dependencies = externalBundleUuids;

        stream->Seek(schemaTableStartOffset);

        // 1. Load Schema Table

        {
            u64 schemaTableSize = 0;
            *stream >> schemaTableSize;

            u32 numSchemaEntries = 0;
            *stream >> numSchemaEntries;

            bundle->schemaTable.Resize(numSchemaEntries);

            for (int i = 0; i < numSchemaEntries; ++i)
            {
                SchemaEntry& entry = bundle->schemaTable[i];

                u32 entryByteSize = 0;
                *stream >> entryByteSize;

                u8 struct1_class0 = 0; // 0 if class ; 1 if struct
                *stream >> struct1_class0;

                entry.isStruct = struct1_class0 == 1;
                entry.isClass = !entry.isStruct;

                String typeName = "";
                *stream >> typeName;

                entry.fullTypeName = typeName;

                u32 classVersion = 0;
                *stream >> classVersion;

                u32 numFields = 0;
                *stream >> numFields;

                entry.fields.Resize(numFields);

                for (int j = 0; j < numFields; ++j)
                {
                    String fieldName = "";
                    *stream >> fieldName;

                    FieldSchema& field = entry.fields[j];

                    field.fieldName = fieldName;

                    u8 typeByte = 0;
                    *stream >> typeByte;

                    field.typeByte = typeByte;

                    if (Optional1Requirement.Exists(typeByte))
                    {
                        u32 schemaIndex = 0;
                        *stream >> schemaIndex;

                        field.schemaIndexOfFieldType = schemaIndex;
                    }

                    if (Optional2Requirement.Exists(typeByte))
                    {
                        u8 underlyingTypeByte = 0;
                        *stream >> underlyingTypeByte;

                        field.underlyingTypeByte = underlyingTypeByte;
                    }
                }
            }
        }

        u32 endOfSchema = 0;
        *stream >> endOfSchema;

        bundle->serializedObjectEntries.Clear();
        bundle->serializedObjectsByUuid.Clear();

        // 2. Serialized Data (Load only the meta-data)

        {
            u64 serializedDataSize = 0;
            *stream >> serializedDataSize;

            u32 numEntries = 0;
            *stream >> numEntries;

            bundle->serializedObjectEntries.Resize(numEntries);

            for (int i = 0; i < numEntries; ++i)
            {
                SerializedObjectEntry& serializedObject = bundle->serializedObjectEntries[i];

                u64 entryByteSize = 0;
                *stream >> entryByteSize;

                u32 dataOffsetAfter = 0;
                *stream >> dataOffsetAfter;

                u64 dataStartOffset = stream->GetCurrentPosition() + (u64)dataOffsetAfter;

                *stream >> serializedObject.instanceUuid;
                *stream >> serializedObject.isAsset;

                *stream >> serializedObject.schemaIndex;

                String pathInBundle;
                *stream >> pathInBundle;
                serializedObject.pathInBundle = pathInBundle;

                String objName;
                *stream >> objName;
                serializedObject.objectName = objName;

                serializedObject.dataStartOffset = dataStartOffset;

                stream->Seek(dataStartOffset);

                u64 byteSizeOfSerializedFields = 0;
                *stream >> byteSizeOfSerializedFields;

                serializedObject.objectSerializedDataSize = byteSizeOfSerializedFields - sizeof(u64);

                stream->Seek(byteSizeOfSerializedFields - sizeof(u64), SeekMode::Current);

                bundle->serializedObjectsByUuid[serializedObject.instanceUuid] = serializedObject;
            }
        }

        if (loadArgs.destroyOutdatedObjects)
        {
	        // TODO: Test this logic
            // Destroy objects that are no longer present in the bundle

            Array<Ref<Object>> objectsToDelete;
            for (auto [uuid, objectWeak] : bundle->loadedObjectsByUuid)
            {
                if (Ref<Object> object = objectWeak.Get())
                {
                    if (!bundle->serializedObjectsByUuid.KeyExists(object->GetUuid()))
                    {
                        // This object no longer exists in the new bundle, so we delete it.
                        objectsToDelete.Add(object);
                    }
                }
            }

            for (Ref<Object> object : objectsToDelete)
            {
                object->BeginDestroy();
            }
            objectsToDelete.Clear();
        }

        if (loadArgs.loadFully && !bundle->isFullyLoaded)
        {
            bundle->isFullyLoaded = true;

            for (const auto& serializedObject : bundle->serializedObjectEntries)
            {
                if (bundle->serializedObjectsByUuid[serializedObject.instanceUuid].isLoaded)
                    continue;

                bundle->LoadObject(stream, serializedObject.instanceUuid);
            }
        }

        return bundle;
    }

    Ref<Object> Bundle::LoadObject(Stream* stream, Uuid objectUuid)
    {
        ZoneScoped;

        if (!serializedObjectsByUuid.KeyExists(objectUuid))
        {
            return nullptr;
        }

        Ref<Object> object = nullptr;

        // 1. Find or instantiate the Object
        {
            LockGuard lock{ loadedObjectsMutex };

            if (loadedObjectsByUuid.KeyExists(objectUuid))
            {
                object = loadedObjectsByUuid[objectUuid].Lock();
            }

            if (object.IsNull())
            {
                int schemaIndex = serializedObjectsByUuid[objectUuid].schemaIndex;
                const auto& schema = schemaTable[schemaIndex];

                ClassType* clazz = ClassType::FindClass(schema.fullTypeName);
                if (clazz != nullptr)
                {
                    Internal::ObjectCreateParams params{};
                    params.objectClass = clazz;
                    params.outer = nullptr;
                    params.uuid = objectUuid;
                    params.objectFlags = OF_NoFlags;
                    params.templateObject = nullptr;
                    params.name = serializedObjectsByUuid[objectUuid].objectName.GetString();

                    object = Internal::CreateObjectInternal(params);

                    loadedObjectsByUuid[objectUuid] = object;
                }
                else
                {
                    return nullptr;
                }
            }
        }

        // 2. Deserialize
        {
            SerializedObjectEntry& serializedObject = serializedObjectsByUuid[objectUuid];

            stream->Seek(serializedObject.dataStartOffset);

            u64 serializedDataSize = 0;
            *stream >> serializedDataSize;

            ObjectSerializer deserializer{ this, object.Get(), serializedObject.schemaIndex };

            deserializer.Deserialize(stream);

            object->OnAfterDeserialize();

            serializedObject.isLoaded = true;
        }

        return object;
    }

    BundleSaveResult Bundle::SaveToDisk(const Ref<Bundle>& bundle, Ref<Object> asset, Stream* stream)
    {
        ZoneScoped;

        if (bundle.IsNull())
        {
            CE_LOG(Error, All, "Failed to save bundle: Bundle is NULL!");
            return BundleSaveResult::InvalidBundle;
        }
        if (bundle->IsTransient())
        {
            CE_LOG(Error, All, "Cannot not save a transient bundle: {}", bundle->GetName());
            return BundleSaveResult::InvalidBundle;
        }
        if (stream == nullptr || !stream->CanWrite())
        {
            CE_LOG(Error, All, "Failed to save bundle: Stream is either NULL or not writeable!");
            return BundleSaveResult::InvalidStream;
        }

        // Prevent bundle Read-Write and Write-Write scenarios.
        LockGuard bundleLock{ bundle->bundleMutex };

        // 1. Fetch all object and struct types

        Array<ClassType*> schemaClasses;
        Array<StructType*> schemaStructs;

        bundle->FetchAllSchemaTypes(schemaClasses, schemaStructs);

        Array<StructType*> schemaTypes;
        HashMap<StructType*, u32> schemaTypeToIndex;

        for (ClassType* schemaClass : schemaClasses)
        {
            schemaTypes.Add(schemaClass);
        }
        schemaTypes.AddRange(schemaStructs);

        for (int i = 0; i < schemaTypes.GetSize(); ++i)
        {
            schemaTypeToIndex[schemaTypes[i]] = (u32)i;
        }

        Array<Object*> objectsToSerialize{};
        if (asset == nullptr)
        {
            asset = bundle;
        }
        bundle->FetchSubObjectsRecursive(objectsToSerialize);

        HashMap<Uuid, Object*> objectReferences{};
        objectReferences.Add({ asset->GetUuid(), asset.Get() });
        asset->FetchObjectReferences(objectReferences);
        Array<Uuid> bundleDependencies{};

        for (const auto& [objectUuid, objectInstance] : objectReferences)
        {
            if (objectInstance == nullptr || objectUuid.IsNull())
                continue;
            if (objectInstance->IsTransient())
                continue;
            auto objectBundle = objectInstance->GetBundle();

            if (bundle != objectBundle && objectBundle != nullptr && !objectBundle->IsTransient())
            {
                bundleDependencies.Add(objectBundle->GetUuid());
            }
        }

        stream->SetBinaryMode(true);

        *stream << BUNDLE_MAGIC_NUMBER;

        u32 headerChecksum = 0;
        *stream << headerChecksum;

        *stream << BUNDLE_VERSION_MAJOR;
        *stream << BUNDLE_VERSION_MINOR;
        *stream << BUNDLE_VERSION_PATCH;

        u64 schemaTableOffset_Location = stream->GetCurrentPosition();
        *stream << (u64)0; // Placeholder
        u64 serializedDataOffset_Location = stream->GetCurrentPosition();
        *stream << (u64)0; // Placeholder

        *stream << bundle->GetUuid();
        *stream << bundle->GetName();

        SaveBundleDependencies(stream, bundleDependencies);

        u8 isCooked = 0;
        *stream << isCooked;

        // - Schema Table -

	    {
		    u64 curLocation = stream->GetCurrentPosition();
        	stream->Seek((s64)schemaTableOffset_Location, SeekMode::Begin);
        	*stream << curLocation;
        	stream->Seek((s64)curLocation, SeekMode::Begin);

        	SerializeSchemaTable(bundle, stream, schemaTypes, schemaTypeToIndex);
	    }

        // - Serialized Data -

        {
            u64 curLocation = stream->GetCurrentPosition();
            stream->Seek((s64)serializedDataOffset_Location, SeekMode::Begin);
            *stream << curLocation;
            stream->Seek((s64)curLocation, SeekMode::Begin);

            u64 serializedDataSize_Location = stream->GetCurrentPosition();
            *stream << (u64)0;

            *stream << (u32)objectsToSerialize.GetSize();

            for (int i = 0; i < objectsToSerialize.GetSize(); ++i)
            {
                ClassType* targetClass = objectsToSerialize[i]->GetClass();
                ObjectSerializer serializer{ bundle, objectsToSerialize[i], (u32)schemaTypeToIndex[targetClass] };

                objectsToSerialize[i]->OnBeforeSerialize();

                serializer.Serialize(stream);
            }

            *stream << (u32)0; // End of data

            curLocation = stream->GetCurrentPosition();
            stream->Seek(serializedDataSize_Location);
            *stream << (curLocation - serializedDataSize_Location);
            stream->Seek(curLocation);
        }

        // - EOF -

        *stream << (u64)0; // End Of File

        return BundleSaveResult::Success;
    }

    bool Bundle::IsFieldSerialized(const Ptr<FieldType>& field, StructType* schemaType)
    {
        if (field == nullptr || schemaType == nullptr || !field->IsSerialized())
            return false;

        TypeId fieldDeclId = field->GetDeclarationTypeId();

        if (field->IsObjectField() || field->IsStructField() || field->IsEnumField())
        {
            return true;
        }

        if (field->IsArrayField())
        {
            TypeId underlyingTypeId = field->GetUnderlyingTypeId();
            if (underlyingTypeId == 0)
            {
                return false;
            }

            TypeInfo* underlyingType = GetTypeInfo(underlyingTypeId);
            if (underlyingType == nullptr)
            {
                return false;
            }

            if (underlyingType->IsEnum())
            {
                return true;
            }

            if (FieldTypeBytes.KeyExists(underlyingTypeId))
            {
                return true;
            }

            if (underlyingType != nullptr)
            {
                return underlyingType->IsObject() || underlyingType->IsStruct();
            }

            return false;
        }

        if (FieldTypeBytes.KeyExists(fieldDeclId))
        {
            return true;
        }

        return false;
    }

    void Bundle::SerializeSchemaTable(const Ref<Bundle>& bundle, Stream* stream, const Array<StructType*>& schemaTypes,
        const HashMap<StructType*, u32>& schemaTypeToIndex)
    {
        u64 tableSize_Location = stream->GetCurrentPosition();
        *stream << (u64)0; // Placeholder

        // Num of schema entries
        *stream << (u32)schemaTypes.GetSize();

        for (int i = 0; i < schemaTypes.GetSize(); ++i)
        {
            StructType* schemaType = schemaTypes[i];

            u64 entrySize_Location = stream->GetCurrentPosition();
            *stream << (u32)0;

            if (schemaType->IsClass())
            {
                *stream << (u8)0; // 0 = Class
            }
            else
            {
                *stream << (u8)1; // 1 = Struct
            }

            *stream << schemaType->GetTypeName().GetString();

            *stream << (u32)0; // Class/struct version (unused)

            Array<Ptr<FieldType>> serializedFields;

            for (int j = 0; j < schemaType->GetFieldCount(); ++j)
            {
                Ptr<FieldType> field = schemaType->GetFieldAt(j);

                if (IsFieldSerialized(field, schemaType))
                {
                    serializedFields.Add(field);
                }
            }

            *stream << (u32)serializedFields.GetSize();

            for (int j = 0; j < serializedFields.GetSize(); ++j)
            {
                *stream << serializedFields[j]->GetName().GetString();

                SerializeFieldSchema(serializedFields[j], stream, schemaTypeToIndex);
            }

            u64 curLocation = stream->GetCurrentPosition();
            stream->Seek(entrySize_Location, SeekMode::Begin);
            *stream << (u32)(curLocation - entrySize_Location);
            stream->Seek(curLocation, SeekMode::Begin);
        }

        *stream << (u32)0; // End of schema table

        {
            u64 curLocation = stream->GetCurrentPosition();
            stream->Seek(tableSize_Location);
            *stream << (u64)(curLocation - tableSize_Location);
            stream->Seek(curLocation);
        }
    }

    void Bundle::SerializeFieldSchema(const Ptr<FieldType>& field, Stream* stream,
                                      const HashMap<StructType*, u32>& schemaTypeToIndex)
    {
        TypeId fieldTypeId = field->GetDeclarationTypeId();
        u8 typeByte = 0;

        if (field->IsObjectField())
        {
            typeByte = FieldTypeBytes[TYPEID(Object)];
            *stream << typeByte;
        }
        else if (field->IsEnumField())
        {
            typeByte = FieldTypeBytes[TYPEID(s64)];
            *stream << typeByte;
        }
        else if (FieldTypeBytes.KeyExists(fieldTypeId))
        {
            typeByte = FieldTypeBytes[fieldTypeId];
            *stream << typeByte;
        }
        else if (field->IsStructField())
        {
            typeByte = StructFieldType;
            *stream << typeByte;

            *stream << (u32)schemaTypeToIndex.Get((StructType*)field->GetDeclarationType());
        }
        else if (field->IsArrayField())
        {
            TypeId underlyingTypeId = field->GetUnderlyingTypeId();
            TypeInfo* underlyingType = field->GetUnderlyingType();

            if (underlyingType->IsObject())
            {
                typeByte = ObjectArrayFieldType;
                *stream << typeByte;
            }
            else if (FieldTypeBytes.KeyExists(underlyingTypeId))
            {
                typeByte = SimpleArrayFieldType;
                *stream << typeByte;

                *stream << FieldTypeBytes[underlyingTypeId];
            }
            else if (underlyingType->IsStruct())
            {
                typeByte = StructArrayFieldType;
                *stream << typeByte;

                *stream << (u32)schemaTypeToIndex.Get((StructType*)underlyingType);
            }
            else if (underlyingType->IsEnum())
            {
                typeByte = SimpleArrayFieldType;
                *stream << typeByte;

                *stream << FieldTypeBytes[TYPEID(s64)];
            }
            else
            {
                String msg = String::Format("Bundle::SaveToDisk(): Failed to serialize field: {}", field->GetName());
                CE_LOG(Error, All, msg);

                throw SerializationException(msg);
            }
        }
        else
        {
            String msg = String::Format("Bundle::SaveToDisk(): Failed to serialize field: {}", field->GetName());
            CE_LOG(Error, All, msg);

            throw SerializationException(msg);
        }
    }

    // --------------------------------------------------------
    // ObjectSerializer

    ObjectSerializer::ObjectSerializer(const Ref<Bundle>& bundle, Object* target, u32 schemaIndex)
	    : bundle(bundle), target(target), schemaIndex(schemaIndex)
    {
	    
    }

    void ObjectSerializer::Serialize(Stream* stream)
    {
        u64 entrySize_Location = stream->GetCurrentPosition();
        *stream << (u64)8;

        u64 dataStartOffset_Location = stream->GetCurrentPosition();
        *stream << (u32)0;

        *stream << target->GetUuid();

        *stream << target->IsAsset();

        ClassType* classType = target->GetClass();

        *stream << schemaIndex;

        *stream << target->GetPathInBundle(bundle.Get());

        *stream << target->GetName();

        // - New header fields here -

        u64 curLocation = stream->GetCurrentPosition();
        stream->Seek(dataStartOffset_Location);
        *stream << (u32)(curLocation - dataStartOffset_Location - 4);
        stream->Seek(curLocation);

        // Data Fields

        u64 sizeOfFieldsSection_Location = stream->GetCurrentPosition();
        *stream << (u64)8; // size of ALL fields in bytes

        for (int i = 0; i < classType->GetFieldCount(); ++i)
        {
            Ptr<FieldType> field = classType->GetFieldAt(i);
            if (!Bundle::IsFieldSerialized(field, classType))
                continue;

            SerializeField(field, target, stream);
        }

        curLocation = stream->GetCurrentPosition();
        stream->Seek(sizeOfFieldsSection_Location, SeekMode::Begin);
        *stream << (u64)(curLocation - sizeOfFieldsSection_Location);
        stream->Seek(curLocation);

        curLocation = stream->GetCurrentPosition();
        stream->Seek(entrySize_Location);
        *stream << (u64)(curLocation - entrySize_Location);
        stream->Seek(curLocation);
    }

    void ObjectSerializer::Deserialize(Stream* stream)
    {
        const auto& schema = bundle->schemaTable[schemaIndex];
        ClassType* classType = target->GetClass();

        for (int i = 0; i < schema.fields.GetSize(); ++i)
        {
            const Bundle::FieldSchema& fieldSchema = schema.fields[i];
            const Name& fieldName = fieldSchema.fieldName;

            Ptr<FieldType> field = classType->FindField(fieldName);

            DeserializeField(field, target, stream, fieldSchema);
        }
    }

    void ObjectSerializer::DeserializeField(const Ptr<FieldType>& field, void* instance, Stream* stream,
        const Bundle::FieldSchema& fieldSchema)
    {
        TypeId fieldTypeId = 0;
        if (field != nullptr)
        {
            fieldTypeId = field->GetTypeId();
        }

        u64 unsignedInteger = 0;
        s64 signedInteger = 0;
        f32 f32Value = 0;
        f64 f64Value = 0;

        switch (fieldSchema.typeByte)
        {
            case 0x01: // u8
            {
                u8 value = 0;
                *stream >> value;
                unsignedInteger = value;
                break;
            }
            case 0x02: // u16
            {
                u16 value = 0;
                *stream >> value;
                unsignedInteger = value;
                break;
            }
            case 0x03: // u32
            {
                u32 value = 0;
                *stream >> value;
                unsignedInteger = value;
                break;
            }
            case 0x04: // u64
            {
                u64 value = 0;
                *stream >> value;
                unsignedInteger = value;
                break;
            }
            case 0x05: // s8
            {
                s8 value = 0;
                *stream >> value;
                signedInteger = value;
                break;
            }
            case 0x06: // s16
            {
                s16 value = 0;
                *stream >> value;
                signedInteger = value;
                break;
            }
            case 0x07: // s32
            {
                s32 value = 0;
                *stream >> value;
                signedInteger = value;
                break;
            }
            case 0x08: // s64
            {
                s64 value = 0;
                *stream >> value;
                signedInteger = value;
                break;
            }
            case 0x09: // f32
            {
                *stream >> f32Value;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(f32))
                    {
                        field->ForceSetFieldValue<f32>(instance, f32Value);
                    }
                    else if (fieldTypeId == TYPEID(f64))
                    {
                        field->ForceSetFieldValue<f64>(instance, f32Value);
                    }
                }
                break;
            }
            case 0x0A: // f64
            {
                *stream >> f64Value;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(f32))
                    {
                        field->ForceSetFieldValue<f32>(instance, (f32)f64Value);
                    }
                    else if (fieldTypeId == TYPEID(f64))
                    {
                        field->ForceSetFieldValue<f64>(instance, f64Value);
                    }
                }
                break;
            }
            case 0x0B:
            {
                bool boolValue = 0;
                *stream >> boolValue;
                if (field != nullptr && fieldTypeId == TYPEID(bool))
                {
                    field->ForceSetFieldValue<bool>(instance, boolValue);
                }
                break;
            }
            case 0x0C:
            {
                String strValue;
                *stream >> strValue;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(String))
                    {
                        field->ForceSetFieldValue<String>(instance, strValue);
                    }
                    else if (fieldTypeId == TYPEID(Name))
                    {
                        field->ForceSetFieldValue<Name>(instance, strValue);
                    }
                    else if (fieldTypeId == TYPEID(IO::Path))
                    {
                        field->ForceSetFieldValue<IO::Path>(instance, strValue);
                    }
                }
                break;
            }
            case 0x0D:
            {
                Vec2 value;
                *stream >> value.x;
                *stream >> value.y;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(Vec2))
                        field->ForceSetFieldValue<Vec2>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec3))
                        field->ForceSetFieldValue<Vec3>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec4))
                        field->ForceSetFieldValue<Vec4>(instance, value);
                }
                break;
            }
            case 0x0E:
            {
                Vec3 value;
                *stream >> value.x;
                *stream >> value.y;
                *stream >> value.z;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(Vec2))
                        field->ForceSetFieldValue<Vec2>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec3))
                        field->ForceSetFieldValue<Vec3>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec4))
                        field->ForceSetFieldValue<Vec4>(instance, value);
                }
                break;
            }
            case 0x0F:
            {
                Vec4 value;
                *stream >> value.x;
                *stream >> value.y;
                *stream >> value.z;
                *stream >> value.w;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(Vec2))
                        field->ForceSetFieldValue<Vec2>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec3))
                        field->ForceSetFieldValue<Vec3>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec4))
                        field->ForceSetFieldValue<Vec4>(instance, value);
                    else if (fieldTypeId == TYPEID(Color))
                        field->ForceSetFieldValue<Color>(instance, value);
                }
                break;
            }
            case 0x10:
            {
                Vec2i value;
                *stream >> value.x;
                *stream >> value.y;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(Vec2i))
                        field->ForceSetFieldValue<Vec2i>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec3i))
                        field->ForceSetFieldValue<Vec3i>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec4))
                        field->ForceSetFieldValue<Vec4i>(instance, value);
                }
                break;
            }
            case 0x11:
            {
                Vec3i value;
                *stream >> value.x;
                *stream >> value.y;
                *stream >> value.z;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(Vec2i))
                        field->ForceSetFieldValue<Vec2i>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec3i))
                        field->ForceSetFieldValue<Vec3i>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec4))
                        field->ForceSetFieldValue<Vec4i>(instance, value);
                }
                break;
            }
            case 0x12:
            {
                Vec4i value;
                *stream >> value.x;
                *stream >> value.y;
                *stream >> value.z;
                *stream >> value.w;
                if (field != nullptr)
                {
                    if (fieldTypeId == TYPEID(Vec2i))
                        field->ForceSetFieldValue<Vec2i>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec3i))
                        field->ForceSetFieldValue<Vec3i>(instance, value);
                    else if (fieldTypeId == TYPEID(Vec4))
                        field->ForceSetFieldValue<Vec4i>(instance, value);
                }
                break;
            }
            case 0x13: // Array of objects
            {
                u32 numElements = 0;
                *stream >> numElements;

                Array<Ptr<FieldType>> elements;
                void* arrayInstance = nullptr;

                if (field != nullptr && field->GetUnderlyingType() != nullptr &&
                    field->GetUnderlyingType()->IsObject())
                {
                    field->ResizeArray(instance, numElements);

                    elements = field->GetArrayFieldListPtr(instance);

                    if (numElements > 0)
                    {
                        const Array<u8>& rawArray = field->GetFieldValue<Array<u8>>(instance);
                        arrayInstance = (void*)rawArray.GetData();
                    }
                }

                for (int i = 0; i < numElements; ++i)
                {
                    DeserializeField(elements.NotEmpty() ? elements[i] : nullptr, arrayInstance, stream, Bundle::FieldSchema{ .typeByte = FieldTypeBytes[TYPEID(Object)] });
                }

                break;
            }
            case 0x14: // Array of structs
            {
                u64 location = stream->GetCurrentPosition();
                u32 numElements = 0;
                *stream >> numElements;

                Array<Ptr<FieldType>> elements;
                void* arrayInstance = nullptr;

                if (field != nullptr && field->GetUnderlyingType() != nullptr &&
                    field->GetUnderlyingType()->IsStruct())
                {
                    field->ResizeArray(instance, numElements);

                    elements = field->GetArrayFieldListPtr(instance);

                    if (numElements > 0)
                    {
                        const Array<u8>& rawArray = field->GetFieldValue<Array<u8>>(instance);
                        arrayInstance = (void*)rawArray.GetData();
                    }
                }

                for (int i = 0; i < numElements; ++i)
                {
                    DeserializeField(elements.NotEmpty() ? elements[i] : nullptr, arrayInstance, stream,
                        Bundle::FieldSchema{ .typeByte = StructFieldType, .schemaIndexOfFieldType = fieldSchema.schemaIndexOfFieldType });
                }

                break;
            }
            case 0x15: // Array of POD types
            {
                u32 numElements = 0;
                *stream >> numElements;

                Array<Ptr<FieldType>> elements;
                void* arrayInstance = nullptr;

                if (field != nullptr && (FieldTypeBytes.KeyExists(field->GetUnderlyingTypeId()) || field->IsEnumArrayField()))
                {
                    field->ResizeArray(instance, numElements);

                    elements = field->GetArrayFieldListPtr(instance);

                    if (numElements > 0)
                    {
                        const Array<u8>& rawArray = field->GetFieldValue<Array<u8>>(instance);
                        arrayInstance = (void*)rawArray.GetData();
                    }
                }

                for (int i = 0; i < numElements; ++i)
                {
                    DeserializeField(elements.NotEmpty() ? elements[i] : nullptr, arrayInstance, stream,
                        Bundle::FieldSchema{ .typeByte = fieldSchema.underlyingTypeByte });
                }

                break;
            }
            case 0x16: // BinaryBlob
            {
                if (field != nullptr && fieldTypeId == TYPEID(BinaryBlob))
                {
                    *stream >> const_cast<BinaryBlob&>(field->GetFieldValue<BinaryBlob>(instance));
                }
                else
                {
                    BinaryBlob buffer;
                    *stream >> buffer;
                }

                break;
            }
            case 0x17: // Object Ref
            {
                Uuid objectUuid;
                *stream >> objectUuid;
                Uuid bundleUuid = Uuid::Zero();

                if (objectUuid.IsValid())
                {
                    *stream >> bundleUuid;
                }

                if (field != nullptr && field->IsObjectField())
                {
                    Ref<Object> referencedObject = LoadObjectReference(objectUuid, bundleUuid);
                    if (field->IsStrongRefCounted())
                    {
                        field->ForceSetFieldValue<Ref<Object>>(instance, referencedObject);
                    }
                    else if (field->IsWeakRefCounted())
                    {
                        field->ForceSetFieldValue<WeakRef<Object>>(instance, referencedObject);
                    }
                    else
                    {
                        field->ForceSetFieldValue<Object*>(instance, referencedObject.Get());
                    }
                }

                break;
            }
            case 0x18: // Function Binding
            {
                Uuid objectUuid;
                *stream >> objectUuid;
                Uuid bundleUuid;
                String functionName;

                if (objectUuid.IsValid())
                {
                    *stream >> bundleUuid;
                    *stream >> functionName;
                }

                if (field != nullptr && field->IsDelegateField() && objectUuid.IsValid() && bundleUuid.IsValid() && functionName.NotEmpty())
                {
                    Ref<Object> referencedObject = LoadObjectReference(objectUuid, bundleUuid);
                    if (referencedObject.IsValid())
                    {
                        IScriptDelegate* delegate = field->GetFieldDelegateValue(instance);
                        Array<FunctionType*> functions = referencedObject->GetClass()->FindAllFunctions(functionName);

                        for (FunctionType* function : functions)
                        {
                            if (delegate->GetSignature() == function->GetFunctionSignature())
                            {
                                delegate->Bind(referencedObject, function);
                                break;
                            }
                        }
                    }
                }

                break;
            }
            case 0x19: // Array of Function Binding
            {
                u32 numBindings = 0;
                *stream >> numBindings;

                for (int i = 0; i < numBindings; ++i)
                {
                    Uuid objectUuid;
                    Uuid bundleUuid;
                    String functionName;

                    *stream >> objectUuid;

                    if (field != nullptr && field->IsEventField() && objectUuid.IsValid())
                    {
                        IScriptEvent* event = field->GetFieldEventValue(instance);
                        event->UnbindAll();
                    }

                    if (objectUuid.IsValid())
                    {
                        *stream >> bundleUuid;
                        *stream >> functionName;

                        if (field != nullptr && field->IsEventField() && objectUuid.IsValid() && bundleUuid.IsValid() && functionName.NotEmpty())
                        {
                            Ref<Object> referencedObject = LoadObjectReference(objectUuid, bundleUuid);
                            if (referencedObject.IsValid())
                            {
                                IScriptEvent* event = field->GetFieldEventValue(instance);
                                Array<FunctionType*> functions = referencedObject->GetClass()->FindAllFunctions(functionName);

                                for (FunctionType* function : functions)
                                {
                                    if (event->GetSignature() == function->GetFunctionSignature())
                                    {
                                        event->Bind(referencedObject, function);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                break;
            }
            case 0x1A: // Struct
            {
                int schemaIndexOfStruct = fieldSchema.schemaIndexOfFieldType;

                const auto& structSchema = bundle->schemaTable[schemaIndexOfStruct];
                StructType* structType = nullptr;
                void* structInstance = nullptr;

                if (field != nullptr && field->IsStructField() && field->GetDeclarationType()->GetTypeName() == structSchema.fullTypeName)
                {
                    structType = (StructType*)field->GetDeclarationType();

                    structInstance = field->GetFieldInstance(instance);
                }

                for (int i = 0; i < structSchema.fields.GetSize(); ++i)
                {
                    Ptr<FieldType> structField = structType != nullptr
                        ? structType->FindField(structSchema.fields[i].fieldName)
                        : nullptr;

                    DeserializeField(structField, structInstance, stream, structSchema.fields[i]);
                }

                if (structType != nullptr && structInstance != nullptr)
                {
                    structType->OnAfterDeserialize(structInstance);
                }

                break;
            }
            case 0x1B: // Uuid
            {
                Uuid uuid;
                *stream >> uuid;

                if (field != nullptr && fieldTypeId == TYPEID(Uuid))
                {
                    field->ForceSetFieldValue<Uuid>(instance, uuid);
                }

                break;
            }
            case 0x1C: // Object Store
            {
                u32 numElements = 0;
                *stream >> numElements;

                if (field != nullptr && field->GetDeclarationTypeId() == TYPEID(ObjectMap))
                {
                    ObjectMap& objectMap = const_cast<ObjectMap&>(field->GetFieldValue<ObjectMap>(instance));

                    objectMap.RemoveAll();
                }

                for (int i = 0; i < numElements; ++i)
                {
                    Uuid objectUuid;
                    Uuid bundleUuid;

                    *stream >> objectUuid;
                    *stream >> bundleUuid;

                    if (field != nullptr && field->GetDeclarationTypeId() == TYPEID(ObjectMap))
                    {
                        ObjectMap& objectMap = const_cast<ObjectMap&>(field->GetFieldValue<ObjectMap>(instance));

                        Ref<Object> referencedObject = LoadObjectReference(objectUuid, bundleUuid);
                        if (referencedObject.IsValid())
                        {
                            objectMap.AddObject(referencedObject.Get());
                        }
                    }
                }

                break;
            }
        }

        if (field != nullptr && fieldSchema.typeByte >= 0x01 && fieldSchema.typeByte <= 0x08)
        {
            if (field->IsEnumField())
                field->SetFieldEnumValue(instance, signedInteger);
            else if (fieldTypeId == TYPEID(u8))
                field->ForceSetFieldValue(instance, (u8)unsignedInteger);
            else if (fieldTypeId == TYPEID(u16))
                field->ForceSetFieldValue(instance, (u16)unsignedInteger);
            else if (fieldTypeId == TYPEID(u32))
                field->ForceSetFieldValue(instance, (u32)unsignedInteger);
            else if (fieldTypeId == TYPEID(u64))
                field->ForceSetFieldValue(instance, (u64)unsignedInteger);
            else if (fieldTypeId == TYPEID(s8))
                field->ForceSetFieldValue(instance, (s8)signedInteger);
            else if (fieldTypeId == TYPEID(s16))
                field->ForceSetFieldValue(instance, (s16)signedInteger);
            else if (fieldTypeId == TYPEID(s32))
                field->ForceSetFieldValue(instance, (s32)signedInteger);
            else if (fieldTypeId == TYPEID(s64))
                field->ForceSetFieldValue(instance, (s64)signedInteger);
            else if (fieldTypeId == TYPEID(f32))
                field->ForceSetFieldValue(instance, (f32)signedInteger);
            else if (fieldTypeId == TYPEID(f64))
                field->ForceSetFieldValue(instance, (f64)signedInteger);
        }
    }

    Ref<Object> ObjectSerializer::LoadObjectReference(Uuid objectUuid, Uuid bundleUuid)
    {
        if (objectUuid.IsNull() || bundleUuid.IsNull())
        {
            return nullptr;
        }

        Ref<Bundle> referencedBundle = nullptr;

        if (bundleUuid == bundle->GetUuid())
        {
            // If the referenced object is in the same bundle as current object
            referencedBundle = bundle;
        }
        else
        {
            LockGuard lock{ Bundle::bundleRegistryMutex };

            if (Bundle::loadedBundlesByUuid.KeyExists(bundleUuid))
            {
                referencedBundle = Bundle::loadedBundlesByUuid[bundleUuid].Lock();
            }
        }

        if (referencedBundle.IsValid())
        {
            return referencedBundle->LoadObject(objectUuid);
        }

        LoadBundleArgs loadArgs{
            .loadFully = false,
            .forceReload = false,
            .destroyOutdatedObjects = true
        };

        Ref<Object> outer = bundle->GetOuter().Lock();

        referencedBundle = Bundle::LoadBundle(outer, bundleUuid, loadArgs);
        if (referencedBundle.IsValid())
        {
            return referencedBundle->LoadObject(objectUuid);
        }

        return nullptr;
    }

#define SERIALIZE_POD_FIELD_CHAIN(PodType) \
    if (fieldTypeId == TYPEID(PodType))\
    {\
        *stream << field->GetFieldValue<PodType>(instance);\
    }

    void ObjectSerializer::SerializeField(const Ptr<FieldType>& field, void* instance, Stream* stream)
    {
        TypeId fieldTypeId = field->GetDeclarationTypeId();

        if (field->IsObjectField())
        {
            Object* object = nullptr;
	        if (field->IsStrongRefCounted())
	        {
                object = field->GetFieldValue<Ref<Object>>(instance).Get();
	        }
            else if (field->IsWeakRefCounted())
            {
                object = field->GetFieldValue<WeakRef<Object>>(instance).Get();
            }
            else
            {
                object = field->GetFieldValue<Object*>(instance);
            }

            Bundle* objectBundle = nullptr;
            if (object)
            {
	            objectBundle = object->GetBundle();
            }

            if (object == nullptr || (objectBundle == nullptr))
            {
                *stream << Uuid::Zero();
            }
            else
            {
                *stream << object->GetUuid();
                *stream << objectBundle->GetUuid();
            }
        }
        else if (field->IsEnumField())
        {
            *stream << field->GetFieldEnumValue(instance);
        }
        else if (FieldTypeBytes.KeyExists(fieldTypeId))
        {
            if (fieldTypeId == TYPEID(ObjectMap))
            {
                const auto& objectStore = field->GetFieldValue<ObjectMap>(instance);

                int objectCounter = 0;
                
                for (int i = 0; i < objectStore.GetObjectCount(); ++i)
                {
                    Ref<Object> object = objectStore.GetObjectAt(i);
                    Ref<Bundle> objectBundle = object->GetBundle();

                    if (object.IsValid() && objectBundle.IsValid() && !objectBundle->IsTransient())
                    {
                        objectCounter++;
                    }
                }
                
                *stream << (u32)objectCounter;

                for (int i = 0; i < objectStore.GetObjectCount(); ++i)
                {
                    Ref<Object> object = objectStore.GetObjectAt(i);
                    Ref<Bundle> objectBundle = object->GetBundle();
                    
                    if (object.IsValid() && objectBundle.IsValid() && !objectBundle->IsTransient())
                    {
                        *stream << object->GetUuid();
                        *stream << objectBundle->GetUuid();
                    }
                }
            }
            else if (fieldTypeId == TYPEID(String) || fieldTypeId == TYPEID(Name) || fieldTypeId == TYPEID(IO::Path))
            {
                String stringValue = field->GetFieldValueAsString(instance);
                *stream << stringValue;
            }
            else SERIALIZE_POD_FIELD_CHAIN(u8)
            else SERIALIZE_POD_FIELD_CHAIN(u16)
            else SERIALIZE_POD_FIELD_CHAIN(u32)
            else SERIALIZE_POD_FIELD_CHAIN(u64)
            else SERIALIZE_POD_FIELD_CHAIN(s8)
            else SERIALIZE_POD_FIELD_CHAIN(s16)
            else SERIALIZE_POD_FIELD_CHAIN(s32)
            else SERIALIZE_POD_FIELD_CHAIN(s64)
            else SERIALIZE_POD_FIELD_CHAIN(f32)
            else SERIALIZE_POD_FIELD_CHAIN(f64)
            else SERIALIZE_POD_FIELD_CHAIN(b8)
            else if (fieldTypeId == CE::GetTypeId<Vec2>())
            {
	            const auto& value = field->GetFieldValue<Vec2>(instance);
                *stream << value.x;
                *stream << value.y;
            }
            else if (fieldTypeId == CE::GetTypeId<Vec2i>())
            {
                const auto& value = field->GetFieldValue<Vec2i>(instance);
                *stream << value.x;
                *stream << value.y;
            }
            else if (fieldTypeId == CE::GetTypeId<Vec3>())
            {
                const auto& value = field->GetFieldValue<Vec3>(instance);
                *stream << value.x;
                *stream << value.y;
                *stream << value.z;
            }
            else if (fieldTypeId == CE::GetTypeId<Vec3i>())
            {
                const auto& value = field->GetFieldValue<Vec3i>(instance);
                *stream << value.x;
                *stream << value.y;
                *stream << value.z;
            }
            else if (fieldTypeId == CE::GetTypeId<Vec4>())
            {
                const auto& value = field->GetFieldValue<Vec4>(instance);
                *stream << value.x;
                *stream << value.y;
                *stream << value.z;
                *stream << value.w;
            }
            else if (fieldTypeId == CE::GetTypeId<Vec4i>())
            {
                const auto& value = field->GetFieldValue<Vec4i>(instance);
                *stream << value.x;
                *stream << value.y;
                *stream << value.z;
                *stream << value.w;
            }
            else if (fieldTypeId == CE::GetTypeId<Color>())
            {
                const auto& value = field->GetFieldValue<Color>(instance);
                *stream << value.r;
                *stream << value.g;
                *stream << value.b;
                *stream << value.a;
            }
            else if (fieldTypeId == TYPEID(BinaryBlob))
            {
                *stream << field->GetFieldValue<BinaryBlob>(instance);
            }
            else if (fieldTypeId == TYPEID(Uuid))
            {
                *stream << field->GetFieldValue<Uuid>(instance);
            }
            else if (fieldTypeId == TYPEID(ScriptDelegate<>))
            {
                IScriptDelegate* scriptDelegate = field->GetFieldDelegateValue(instance);
                FunctionBinding binding = scriptDelegate->GetBinding();

                Ref<Bundle> bundle = nullptr;

                if (Ref<Object> boundObject = binding.object.Lock())
                {
                    bundle = boundObject->GetBundle();

                    if (bundle.IsValid() && binding.function != nullptr)
                    {
                        *stream << boundObject->GetUuid();
                        *stream << bundle->GetUuid();
                        *stream << binding.function->GetName();
                    }
                    else
                    {
                        *stream << Uuid::Zero();
                    }
                }
                else
                {
                    *stream << Uuid::Zero();
                }
            }
            else if (fieldTypeId == TYPEID(ScriptEvent<>))
            {
                IScriptEvent* scriptEvent = field->GetFieldEventValue(instance);

                Array<FunctionBinding> bindings = scriptEvent->GetSerializableBindings();

                *stream << (u32)bindings.GetSize();

                for (const auto& binding : bindings)
                {
                    if (Ref<Object> boundObject = binding.object.Lock())
                    {
                        bundle = boundObject->GetBundle();

                        if (bundle.IsValid() && binding.function != nullptr)
                        {
                            *stream << boundObject->GetUuid();
                            *stream << bundle->GetUuid();
                            *stream << binding.function->GetName();
                        }
                        else
                        {
                            *stream << Uuid::Zero();
                        }
                    }
                    else
                    {
                        *stream << Uuid::Zero();
                    }
                }
            }
        }
        else if (field->IsStructField())
        {
            StructType* structType = (StructType*)field->GetDeclarationType();
            void* structInstance = field->GetFieldInstance(instance);

            if (structType != nullptr && structInstance != nullptr)
            {
                structType->OnBeforeSerialize(structInstance);
            }

            for (int i = 0; i < structType->GetFieldCount(); ++i)
            {
                Ptr<FieldType> structField = structType->GetFieldAt(i);

                if (!Bundle::IsFieldSerialized(structField, structType))
                {
                    continue;
                }

                SerializeField(structField, structInstance, stream);
            }
        }
        else if (field->IsArrayField())
        {
            TypeId underlyingTypeId = field->GetUnderlyingTypeId();
            TypeInfo* underlyingType = field->GetUnderlyingType();

            Array<Ptr<FieldType>> elementList = field->GetArrayFieldListPtr(instance);

            *stream << (u32)elementList.GetSize();

            if (elementList.NotEmpty())
            {
                const Array<u8>& rawArray = field->GetFieldValue<Array<u8>>(instance);
                auto arrayInstance = (void*)&rawArray[0];

                if (underlyingType->IsObject()) // Array of objects
                {
	                for (int i = 0; i < elementList.GetSize(); ++i)
	                {
                        SerializeField(elementList[i], arrayInstance, stream);
	                }
                }
                else if (FieldTypeBytes.KeyExists(underlyingTypeId))
                {
                    for (int i = 0; i < elementList.GetSize(); ++i)
                    {
                        SerializeField(elementList[i], arrayInstance, stream);
                    }
                }
                else if (underlyingType->IsStruct())
                {
                    for (int i = 0; i < elementList.GetSize(); ++i)
                    {
                        SerializeField(elementList[i], arrayInstance, stream);
                    }
                }
                else if (underlyingType->IsEnum())
                {
                    for (int i = 0; i < elementList.GetSize(); ++i)
                    {
                        SerializeField(elementList[i], arrayInstance, stream);
                    }
                }
                else
                {
                    String msg = String::Format("Bundle::SaveToDisk(): Failed to serialize field: {}", field->GetName());
                    CE_LOG(Error, All, msg);

                    throw SerializationException(msg);
                }
            }
        }
    }

} // namespace CE
