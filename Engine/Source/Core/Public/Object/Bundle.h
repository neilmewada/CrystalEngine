#pragma once

namespace CE
{
#if CE_EDITOR_BUILD
    namespace Editor { class AssetImportJob; }
#endif

    enum class BundleLoadResult
    {
        Success = 0,
        UnknownError,
        InvalidStream,
        BundleNotFound,
        InvalidBundle,
        UnsupportedBundleVersion,
    };

    enum class BundleSaveResult
	{
		Success = 0,
		UnknownError,
		InvalidStream,
		InvalidPath,
		InvalidBundle,
		AssetNotInBundle,
	};

    class SerializationException : public Exception
    {
    public:

        SerializationException() : Exception("Serialization Error")
		{}

        SerializationException(const String& msg) : Exception("Serialization Error: " + msg)
		{}
    };

    struct LoadBundleArgs
    {
        //! @brief Loads the bundle as a "temporary" one. Useful for when loading a duplicate of a bundle when
        //! importing an Asset which will then be destroyed immediately after the import.
        bool loadTemporary = false;

        bool loadFully = true;

        //! @brief If the bundle should forcefully be deserialized even if it is already fully loaded.
        bool forceReload = false;

        //! @brief If objects that are no longer part of the bundle should be destroyed.
        //! This might happen when you load a bundle again after it was fully loaded.
        bool destroyOutdatedObjects = true;
    };

    struct IBundleResolver
    {
        virtual ~IBundleResolver() = default;

        virtual Name ResolveBundlePath(const Uuid& uuid) = 0;

    };

    class CORE_API Bundle : public Object
    {
        CE_CLASS(Bundle, Object)
    public:

        struct ObjectData
        {
            Name name;
            Name typeName;
            Uuid uuid;
            Name pathInBundle;
        };

        // - Static API -

        static u32 GetCurrentMajor();
        static u32 GetCurrentMinor();
        static u32 GetCurrentPatch();

        static void PushBundleResolver(IBundleResolver* resolver);
        static void PopBundleResolver(IBundleResolver* resolver);

        static IO::Path GetAbsoluteDirectoryPath(const Name& localPath);
        static IO::Path GetAbsoluteBundlePath(const Name& bundlePath);
        static Name GetBundlePath(const IO::Path& absoluteBundlePath);

        static Ref<Bundle> LoadBundle(const Ref<Object>& outer, const Uuid& bundleUuid, const LoadBundleArgs& loadArgs = LoadBundleArgs());

        static Ref<Bundle> LoadBundleAbsolute(const Ref<Object>& outer, const IO::Path& absolutePath, const LoadBundleArgs& loadArgs = LoadBundleArgs());

        static Ref<Bundle> LoadBundle(const Ref<Object>& outer, const Name& path, const LoadBundleArgs& loadArgs = LoadBundleArgs());
        static Ref<Bundle> LoadBundle(const Ref<Object>& outer, const Name& path, BundleLoadResult& outResult, const LoadBundleArgs& loadArgs = LoadBundleArgs());
        static Ref<Bundle> LoadBundleAbsolute(const Ref<Object>& outer, const IO::Path& absolutePath, BundleLoadResult& outResult, const LoadBundleArgs& loadArgs = LoadBundleArgs());

        static BundleSaveResult SaveToDisk(const Ref<Bundle>& bundle, Ref<Object> asset = nullptr);
        static BundleSaveResult SaveToDisk(const Ref<Bundle>& bundle, Ref<Object> asset, const CE::Name& bundlePath);
        static BundleSaveResult SaveToDisk(const Ref<Bundle>& bundle, Ref<Object> asset, const IO::Path& fullPath);

        // - Public API -

        const Name& GetBundlePath() const { return bundlePath; }

        Ref<Object> LoadObject(Uuid objectUuid);

        Ref<Object> LoadObject(const Name& pathInBundle);

        Ref<Object> LoadObject(ClassType* objectClass);

        template<typename TObject>
        Ref<TObject> LoadObject()
        {
            return (Ref<TObject>)LoadObject(TObject::StaticClass());
        }

        void SetObjectUuid(const Ref<Object>& object, const Uuid& uuid);

        ObjectData GetPrimaryObjectData();

        const Name& GetSourceAssetPath() const { return sourceAssetRelativePath; }

        void DestroyAllSubObjects();

        bool IsFullyLoaded() const { return isFullyLoaded; }

        void LoadFully();

    protected:

        Ref<Object> LoadObject(Stream* stream, Uuid objectUuid);

        static Ref<Bundle> LoadBundle(const Ref<Object>& outer, Stream* stream, BundleLoadResult& outResult, const LoadBundleArgs& loadArgs = LoadBundleArgs());
        static BundleSaveResult SaveToDisk(const Ref<Bundle>& bundle, Ref<Object> asset, Stream* stream);

        void OnAfterConstruct() override;
        void OnBeginDestroy() override;

    private:

        Bundle();

        void OnObjectUnloaded(Object* object);

        static bool IsFieldSerialized(const Ptr<FieldType>& field, StructType* schemaType);

        static void SerializeSchemaTable(const Ref<Bundle>& bundle, Stream* stream, const Array<StructType*>& schemaTypes, 
            const HashMap<StructType*, u32>& schemaTypeToIndex);

        static void SerializeFieldSchema(const Ptr<FieldType>& field, Stream* stream, const HashMap<StructType*, u32>& schemaTypeToIndex);

        void FetchAllSchemaTypes(Array<ClassType*>& outClasses, Array<StructType*>& outStructs);

        struct FieldSchema
        {
            Name fieldName{};
            u8 typeByte = 0;
            u32 schemaIndexOfFieldType = 0;
            u8 underlyingTypeByte = 0;
        };

        struct SchemaEntry
        {
            bool isStruct = false;
            bool isClass = false;
            Name fullTypeName = "";
            Array<FieldSchema> fields{};
        };

        struct SerializedObjectEntry
        {
            u64 dataStartOffset = 0;
            Uuid instanceUuid = Uuid::Null();
            b8 isAsset = false;
            u32 schemaIndex = 0;
			Name pathInBundle{};
            Name objectName{};
            u64 objectSerializedDataSize = 0;

            b8 isLoaded = false;
            b8 isDeserialized = false;
        };

        IO::Path absoluteBundlePath{};
        Name bundlePath{};
        Stream* readerStream = nullptr;

        Name sourceAssetRelativePath{};

        u32 majorVersion = 0;
        u32 minorVersion = 0;

        // If this bundle was created from deserialization
        b8 isLoadedFromDisk = false;
        b8 isFullyLoaded = false;

        Array<SchemaEntry> schemaTable;
        Array<Uuid> dependencies;

        Array<SerializedObjectEntry> serializedObjectEntries;
        HashMap<Uuid, SerializedObjectEntry> serializedObjectsByUuid;

        // Used to prevent simultaneous bundle Load and Store operations.
        SharedMutex bundleMutex{};

        SharedMutex loadedObjectsMutex{};
        HashMap<Uuid, WeakRef<Object>> loadedObjectsByUuid{};

        static SharedRecursiveMutex bundleRegistryMutex;
        static HashMap<Uuid, WeakRef<Bundle>> loadedBundlesByUuid;

        static Array<IBundleResolver*> bundleResolvers;

        friend class ObjectSerializer;
        friend class Object;

        friend class AssetRegistry;
#if CE_EDITOR_BUILD
        friend class CE::Editor::AssetImportJob;
#endif
    };

}

CE_RTTI_CLASS(CORE_API, CE, Bundle,
	CE_SUPER(CE::Object),
	CE_NOT_ABSTRACT,
	CE_ATTRIBS(),
	CE_FIELD_LIST(

    ),
	CE_FUNCTION_LIST()
)
