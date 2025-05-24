
#include "Core.h"

#include "Bundle.inl"

namespace CE
{
    SharedRecursiveMutex Bundle::bundleRegistryMutex{};
    HashMap<Uuid, WeakRef<Bundle>> Bundle::loadedBundlesByUuid{};

    Array<IBundleResolver*> Bundle::bundleResolvers{};

    Bundle::Bundle()
    {

    }

    void Bundle::OnAfterConstruct()
    {
        Super::OnAfterConstruct();

        if (!HasAnyObjectFlags(OF_TemporaryBundleInstance)) // If NOT a temporary bundle instance
	    {
		    LockGuard lock{ bundleRegistryMutex };
        	loadedBundlesByUuid[GetUuid()] = this;
	    }

        {
            LockGuard lock{ loadedObjectsMutex };
            loadedObjectsByUuid[GetUuid()] = this;
        }
    }

    void Bundle::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

	    {
		    LockGuard lock{ bundleRegistryMutex };
	    	loadedBundlesByUuid.Remove(GetUuid());
	    }

        {
            LockGuard lock{ loadedObjectsMutex };
            loadedObjectsByUuid.Remove(GetUuid());
        }
    }

    void Bundle::OnObjectUnloaded(Object* object)
    {
        if (object != nullptr)
        {
            LockGuard lock{ loadedObjectsMutex };
            loadedObjectsByUuid.Remove(object->GetUuid());
        }
    }

    u32 Bundle::GetCurrentMajor()
    {
        return BUNDLE_VERSION_MAJOR;
    }

    u32 Bundle::GetCurrentMinor()
    {
        return BUNDLE_VERSION_MINOR;
    }

    u32 Bundle::GetCurrentPatch()
    {
        return BUNDLE_VERSION_PATCH;
    }

    void Bundle::PushBundleResolver(IBundleResolver* resolver)
    {
        bundleResolvers.Add(resolver);
    }

    void Bundle::PopBundleResolver(IBundleResolver* resolver)
    {
        bundleResolvers.Remove(resolver);
    }

    IO::Path Bundle::GetAbsoluteDirectoryPath(const Name& bundlePath)
    {
        if (!bundlePath.IsValid())
            return {};
        String bundleNameStr = bundlePath.GetString();

        if (bundleNameStr.StartsWith("/Engine/") || bundleNameStr == "/Engine") // Example: /Engine/Assets/Textures/Noise/Perlin/Perlin04
        {
            if (!gProjectPath.Exists() || !(gProjectPath / "Engine/Assets").Exists())
            {
                return EngineDirectories::GetEngineInstallDirectory() / (bundleNameStr.GetSubstring(1));
            }
            return gProjectPath / bundleNameStr.GetSubstring(1);
        }
        else if (bundleNameStr.StartsWith("/Game/") || bundleNameStr == "/Game") // Example: /Game/Assets/Textures/SomeTexture
        {
            return gProjectPath / bundleNameStr.GetSubstring(1);
        }
        else if (bundleNameStr.StartsWith("/Editor/") || bundleNameStr == "/Editor")
        {
            return EngineDirectories::GetEngineInstallDirectory() / bundleNameStr.GetSubstring(1);
        }
        else if (bundleNameStr.StartsWith("/Temp/") || bundleNameStr == "/Temp")
        {
            return gProjectPath / (bundleNameStr.GetSubstring(1) + ".temp");
        }

        if (bundleNameStr[0] != '/')
        {
            bundleNameStr.InsertAt('/', 0);
        }

        return PlatformDirectories::GetLaunchDir() / bundleNameStr.GetSubstring(1);
    }

    IO::Path Bundle::GetAbsoluteBundlePath(const Name& bundlePath)
    {
        if (!bundlePath.IsValid())
            return {};
        String bundleNameStr = bundlePath.GetString();

        if (bundleNameStr.StartsWith("/Engine/") || bundleNameStr == "/Engine") // Example: /Engine/Assets/Textures/Noise/Perlin/Perlin04
        {
            if (!gProjectPath.Exists() || !(gProjectPath / "Engine/Assets").Exists())
            {
                return EngineDirectories::GetEngineInstallDirectory() / (bundleNameStr.GetSubstring(1) + ".casset");
            }
            return gProjectPath / (bundleNameStr.GetSubstring(1) + ".casset");
        }
        else if (bundleNameStr.StartsWith("/Game/") || bundleNameStr == "/Game") // Example: /Game/Assets/Textures/SomeTexture
        {
            return gProjectPath / (bundleNameStr.GetSubstring(1) + ".casset");
        }
        else if (bundleNameStr.StartsWith("/Editor/") || bundleNameStr == "/Editor")
        {
            return EngineDirectories::GetEngineInstallDirectory() / (bundleNameStr.GetSubstring(1) + ".casset");
        }
        else if (bundleNameStr.StartsWith("/Temp/") || bundleNameStr == "/Temp")
        {
            return gProjectPath / (bundleNameStr.GetSubstring(1) + ".temp");
        }

        if (bundleNameStr[0] != '/')
        {
            bundleNameStr.InsertAt('/', 0);
        }

        return PlatformDirectories::GetLaunchDir() / (bundleNameStr.GetSubstring(1) + ".casset");
    }

    Name Bundle::GetBundlePath(const IO::Path& absoluteBundlePath)
    {
        if (absoluteBundlePath.IsEmpty())
            return {};

        if (IO::Path::IsSubDirectory(absoluteBundlePath, gProjectPath))
        {
            String relativePath = IO::Path::GetRelative(absoluteBundlePath, gProjectPath)
                .RemoveExtension().GetString().Replace({ '\\' }, '/');
            if (relativePath[0] != '/')
                relativePath.InsertAt('/', 0);

            return relativePath;
        }
        else if (IO::Path::IsSubDirectory(absoluteBundlePath, EngineDirectories::GetEngineInstallDirectory()))
        {
            String relativePath = IO::Path::GetRelative(absoluteBundlePath, EngineDirectories::GetEngineInstallDirectory())
                .RemoveExtension().GetString().Replace({ '\\' }, '/');
            if (relativePath[0] != '/')
                relativePath.InsertAt('/', 0);

            return relativePath;
        }
        else if (IO::Path::IsSubDirectory(absoluteBundlePath, EngineDirectories::GetGameInstallDirectory()))
        {
            String relativePath = IO::Path::GetRelative(absoluteBundlePath, EngineDirectories::GetGameInstallDirectory())
                .RemoveExtension().GetString().Replace({ '\\' }, '/');
            if (relativePath[0] != '/')
                relativePath.InsertAt('/', 0);

            return relativePath;
        }

        return nullptr;
    }

    Ref<Bundle> Bundle::LoadBundle(const Ref<Object>& outer, const Uuid& bundleUuid, const LoadBundleArgs& loadArgs)
    {
        {
            LockGuard lock{ bundleRegistryMutex };

            if (loadedBundlesByUuid.KeyExists(bundleUuid))
            {
                if (Ref<Bundle> bundle = loadedBundlesByUuid[bundleUuid].Lock())
                {
                    return bundle;
                }
            }
        }

        if (bundleResolvers.IsEmpty())
            return nullptr;

        for (int i = bundleResolvers.GetSize() - 1; i >= 0; --i)
        {
            Name bundlePath = bundleResolvers[i]->ResolveBundlePath(bundleUuid);

            Ref<Bundle> bundle = LoadBundle(outer, bundlePath, loadArgs);
            if (bundle.IsValid())
            {
                return bundle;
            }
        }

        return nullptr;
    }

    Ref<Bundle> Bundle::LoadBundleAbsolute(const Ref<Object>& outer, const IO::Path& absolutePath,
        const LoadBundleArgs& loadArgs)
    {
        BundleLoadResult result;
        return LoadBundleAbsolute(outer, absolutePath, result, loadArgs);
    }

    Ref<Bundle> Bundle::LoadBundle(const Ref<Object>& outer, const Name& path, const LoadBundleArgs& loadArgs)
    {
        BundleLoadResult result;
        return LoadBundle(outer, path, result, loadArgs);
    }

    Ref<Bundle> Bundle::LoadBundle(const Ref<Object>& outer, const Name& path, BundleLoadResult& outResult, const LoadBundleArgs& loadArgs)
    {
        IO::Path absolutePath = GetAbsoluteBundlePath(path);
        return LoadBundleAbsolute(outer, absolutePath, outResult, loadArgs);
    }

    Ref<Bundle> Bundle::LoadBundleAbsolute(const Ref<Object>& outer, const IO::Path& absolutePath, BundleLoadResult& outResult,
        const LoadBundleArgs& loadArgs)
    {
        if (!absolutePath.Exists())
        {
            CE_LOG(Error, All, "Bundle::LoadFromDisk(): Bundle doesn't exist at path {}", absolutePath);
            outResult = BundleLoadResult::BundleNotFound;
            return nullptr;
        }

        FileStream stream = FileStream(absolutePath, Stream::Permissions::ReadOnly);
        stream.SetBinaryMode(true);

        Ref<Bundle> bundle = LoadBundle(outer, &stream, outResult, loadArgs);
        if (bundle.IsValid())
        {
            bundle->absoluteBundlePath = absolutePath;
            bundle->bundlePath = Bundle::GetBundlePath(absolutePath);
        }
        return bundle;
    }

    BundleSaveResult Bundle::SaveToDisk(const Ref<Bundle>& bundle, Ref<Object> asset)
    {
        if (bundle == nullptr)
        {
            CE_LOG(Error, All, "SaveToDisk() with a NULL bundle!");
            return BundleSaveResult::InvalidBundle;
        }

        if (bundle->IsTransient())
        {
            CE_LOG(Error, All, "SaveToDisk() passed with a transient bundle named: {}", bundle->GetName());
            return BundleSaveResult::InvalidBundle;
        }

        IO::Path absolutePath = GetAbsoluteBundlePath(bundle->GetName());

        if (!bundle->absoluteBundlePath.IsEmpty() && bundle->absoluteBundlePath.GetParentPath().Exists())
        {
            absolutePath = bundle->absoluteBundlePath;
        }

        return SaveToDisk(bundle, asset, absolutePath);
    }

    BundleSaveResult Bundle::SaveToDisk(const Ref<Bundle>& bundle, Ref<Object> asset, const CE::Name& bundlePath)
    {
        if (bundle == nullptr)
        {
            CE_LOG(Error, All, "SaveToDisk() with a NULL bundle!");
            return BundleSaveResult::InvalidBundle;
        }

        if (bundle->IsTransient())
        {
            CE_LOG(Error, All, "SaveToDisk() passed with a transient bundle named: {}", bundle->GetName());
            return BundleSaveResult::InvalidBundle;
        }

        IO::Path absolutePath = GetAbsoluteBundlePath(bundlePath);
        return SaveToDisk(bundle, asset, absolutePath);
    }

    BundleSaveResult Bundle::SaveToDisk(const Ref<Bundle>& bundle, Ref<Object> asset, const IO::Path& fullPath)
    {
        auto path = fullPath;
        if (path.GetExtension().IsEmpty())
        {
            path = path.GetString() + ".casset";
        }

        if (!path.GetParentPath().Exists())
        {
            IO::Path::CreateDirectories(path.GetParentPath());
        }

        if (bundle == nullptr)
        {
            CE_LOG(Error, All, "SaveToDisk() with a NULL bundle!");
            return BundleSaveResult::InvalidBundle;
        }

        if (bundle->IsTransient())
        {
            CE_LOG(Error, All, "SaveToDisk() passed with a transient bundle named: {}", bundle->GetName());
            return BundleSaveResult::InvalidBundle;
        }

        if (path.IsDirectory())
        {
            CE_LOG(Error, All, "SaveToDisk() passed with a bundle path that is a directory!");
            return BundleSaveResult::InvalidPath;
        }
        if (asset != nullptr && asset->GetBundle() != bundle.Get())
        {
            CE_LOG(Error, All, "SaveToDisk() passed with an asset object that is not part of the bundle!");
            return BundleSaveResult::AssetNotInBundle;
        }

        bundle->absoluteBundlePath = path;
        bundle->bundlePath = Bundle::GetBundlePath(path);

        FileStream stream = FileStream(path, Stream::Permissions::WriteOnly, true, true);
        stream.SetBinaryMode(true);

        return SaveToDisk(bundle, asset, &stream);
    }

    void Bundle::LoadFully()
    {
        if (isFullyLoaded)
            return;

        LockGuard lock{ bundleMutex };

        for (const auto& serializedObject : serializedObjectEntries)
        {
            LoadObject(serializedObject.instanceUuid);
        }
    }

    Ref<Object> Bundle::LoadObject(Uuid objectUuid)
    {
        if (objectUuid.IsNull())
        {
            return nullptr;
        }

        {
            LockGuard lock{ loadedObjectsMutex };

            if (loadedObjectsByUuid.KeyExists(objectUuid))
            {
                if (Ref<Object> object = loadedObjectsByUuid[objectUuid].Lock())
                {
                    return object;
                }
            }
        }

        if (readerStream != nullptr)
        {
            u64 curPos = readerStream->GetCurrentPosition();
            Ref<Object> retVal = LoadObject(readerStream, objectUuid);
            readerStream->Seek(curPos, SeekMode::Begin);
            return retVal;
        }

        if (absoluteBundlePath.Exists())
        {
            FileStream stream = FileStream(absoluteBundlePath, Stream::Permissions::ReadOnly);
            stream.SetBinaryMode(true);

            readerStream = &stream;

            Ref<Object> object = LoadObject(&stream, objectUuid);

            readerStream = nullptr;
            return object;
        }

        return nullptr;
    }

    Ref<Object> Bundle::LoadObject(const Name& pathInBundle)
    {
        if (!pathInBundle.IsValid())
        {
            return nullptr;
        }

        {
            LockGuard lock{ loadedObjectsMutex };

            for (const auto& [_, objectRef] : loadedObjectsByUuid)
            {
                if (Ref<Object> object = objectRef.Lock())
                {
                    if (object->GetPathInBundle(this) == pathInBundle)
                    {
                        return object;
                    }
                }
            }
        }

        Uuid objectUuid = Uuid::Zero();

        for (const auto& serializedObjectEntry : serializedObjectEntries)
        {
            if (pathInBundle == serializedObjectEntry.pathInBundle)
            {
                objectUuid = serializedObjectEntry.instanceUuid;
                break;
            }
        }

        if (objectUuid.IsNull())
        {
            return nullptr;
        }

        return LoadObject(objectUuid);
    }

    Ref<Object> Bundle::LoadObject(ClassType* objectClass)
    {
        if (objectClass == nullptr)
            return nullptr;

        {
            LockGuard lock{ loadedObjectsMutex };

            for (const auto& [_, objectRef] : loadedObjectsByUuid)
            {
	            if (Ref<Object> object = objectRef.Lock())
	            {
		            if (object->GetClass() == objectClass)
		            {
                        return object;
		            }
	            }
            }
        }

        for (const auto& serializedObjectEntry : serializedObjectEntries)
        {
            if (schemaTable[serializedObjectEntry.schemaIndex].fullTypeName == objectClass->GetTypeName())
            {
                return LoadObject(serializedObjectEntry.instanceUuid);
            }
        }

        return nullptr;
    }

    void Bundle::SetObjectUuid(const Ref<Object>& object, const Uuid& newUuid)
    {
        if (object == nullptr)
            return;

        Uuid oldUuid = object->GetUuid();

        {
            LockGuard lock{ loadedObjectsMutex };

            if (serializedObjectsByUuid.KeyExists(oldUuid))
            {
                serializedObjectsByUuid[newUuid] = serializedObjectsByUuid[oldUuid];
                serializedObjectsByUuid.Remove(oldUuid);

                serializedObjectsByUuid[newUuid].instanceUuid = newUuid;
            }

            if (loadedObjectsByUuid.KeyExists(oldUuid))
            {
                WeakRef<Object> oldRef = loadedObjectsByUuid[oldUuid];
                loadedObjectsByUuid[newUuid] = oldRef.Get();
                loadedObjectsByUuid.Remove(oldUuid);
            }
        }

        if (object->IsBundle())
        {
            LockGuard lock{ bundleRegistryMutex };

            Ref<Bundle> bundle = (Ref<Bundle>)object;

            if (loadedBundlesByUuid.KeyExists(oldUuid))
            {
                WeakRef<Bundle> oldRef = loadedBundlesByUuid[oldUuid];
                loadedBundlesByUuid[newUuid] = oldRef;
                loadedBundlesByUuid.Remove(oldUuid);
            }
        }

        object->uuid = newUuid;
    }

    Bundle::ObjectData Bundle::GetPrimaryObjectData()
    {
        ObjectData defaultData = ObjectData();

        for (const auto& serializedObject : serializedObjectEntries)
        {
            if (!serializedObject.pathInBundle.GetString().Contains('.'))
            {
                // A top-level object
                if (serializedObject.isAsset)
                {
                    return ObjectData{
                        .name = serializedObject.objectName,
                        .typeName = schemaTable[serializedObject.schemaIndex].fullTypeName,
                        .uuid = serializedObject.instanceUuid,
                        .pathInBundle = serializedObject.pathInBundle,
                    };
                }

                if (!defaultData.name.IsValid())
                {
                    defaultData.name = serializedObject.objectName;
                    defaultData.uuid = serializedObject.instanceUuid;
                    defaultData.pathInBundle = serializedObject.pathInBundle;
                    defaultData.typeName = schemaTable[serializedObject.schemaIndex].fullTypeName;
                }
            }
        }

        return defaultData;
    }

    void Bundle::DestroyAllSubObjects()
    {
        for (int i = 0; i < GetSubObjectCount(); i++)
        {
            Ref<Object> subobject = GetSubObject(i);
            DetachSubobject(subobject.Get());
            subobject->BeginDestroy();
        }

        LockGuard lock{ loadedObjectsMutex };

        serializedObjectEntries.Clear();
        serializedObjectsByUuid.Clear();
        loadedObjectsByUuid.Clear();

        dependencies.Clear();
    }

    void Bundle::FetchAllSchemaTypes(Array<ClassType*>& outClasses, Array<StructType*>& outStructs)
    {
        HashSet<ClassType*> classes;
        HashSet<StructType*> structs;

        std::function<void(StructType*)> fetchInternalStructs = [&](StructType* structType)
            {
                if (structType == nullptr || structs.Exists(structType))
                    return;

                structs.Add(structType);

                for (int i = 0; i < structType->GetFieldCount(); ++i)
                {
                    FieldType* field = structType->GetFieldAt(i);

                    if (field->IsStructField())
                    {
                        Struct* childStruct = StructType::FindStruct(field->GetDeclarationTypeId());

                        fetchInternalStructs(childStruct);
                    }
                    else if (field->IsArrayField())
                    {
                        TypeInfo* underlyingType = field->GetUnderlyingType();
                        if (underlyingType != nullptr && underlyingType->IsStruct())
                        {
                            StructType* underlyingStructType = static_cast<StructType*>(underlyingType);

                            fetchInternalStructs(underlyingStructType);
                        }
                    }
                }
            };

        std::function<void(Object*)> fetchStructs = [&](Object* object)
            {
                if (object == nullptr)
                    return;

                Class* clazz = object->GetClass();

                for (int i = 0; i < clazz->GetFieldCount(); ++i)
                {
                    FieldType* field = clazz->GetFieldAt(i);

                    if (field->IsStructField())
                    {
                        Struct* structType = StructType::FindStruct(field->GetDeclarationTypeId());

                        fetchInternalStructs(structType);
                    }
                    else if (field->IsArrayField())
                    {
                        TypeInfo* underlyingType = field->GetUnderlyingType();
                        if (underlyingType != nullptr && underlyingType->IsStruct())
                        {
                            StructType* structType = static_cast<StructType*>(underlyingType);

                            fetchInternalStructs(structType);
                        }
                    }
                }
            };

        std::function<void(Object*)> visitor = [&](Object* object)
            {
                if (object == nullptr)
                    return;

				if (!classes.Exists(object->GetClass()))
		        {
			        classes.Add(object->GetClass());

                	fetchStructs(object);
		        }

                for (int i = 0; i < object->GetSubObjectCount(); ++i)
                {
                    visitor(object->GetSubObject(i));
                }
            };

        visitor(this);

        for (ClassType* clazz : classes)
        {
            outClasses.Add(clazz);
        }

        for (StructType* structType : structs)
        {
            outStructs.Add(structType);
        }
    }
    
} // namespace CE

CE_RTTI_CLASS_IMPL(CORE_API, CE, Bundle)
