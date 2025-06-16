#pragma once

#include "CoreTypes.h"
#include "Containers/String.h"

#include "ObjectMacros.h"
#include "RTTI/RTTIDefines.h"
#include "RTTI/RTTI.h"
#include "RTTI/Variant.h"
#include "Function.h"
#include "Class.h"
#include "Enum.h"
#include "SubClassType.h"

#include "ObjectCreationContext.h"
#include "ObjectGlobals.h"
#include "ObjectMap.h"

#include "Event.h"
#include "Binding.h"

#include "ObjectListener.h"

#if PAL_TRAIT_BUILD_TESTS
class Bundle_WriteRead_Test;
#endif

namespace CE
{

    class ClassType;
    class FieldType;
    class FunctionType;
    class Bundle;

    class CORE_API Object
    {
        CE_CLASS(Object)
    protected:
        Object();
        
        virtual ~Object();

    private:

        void ConstructInternal();
        void ConstructInternal(Internal::ObjectCreateParams* initializer);

    public:

        // - Getters & Setters -
        const Name& GetName() const
        {
            return name;
        }

        void SetName(const Name& newName);
        
        INLINE Uuid GetUuid() const
        {
            return uuid;
        }

        INLINE TypeId GetTypeId() const
        {
            return GetType()->GetTypeId();
        }

        INLINE ObjectFlags GetFlags() const
        {
            return objectFlags;
        }

		INLINE const WeakRef<Object>& GetOuter() const
		{
			return outer;
		}

		bool ParentExistsInChain(Object* parent) const;

        INLINE bool HasAllObjectFlags(ObjectFlags flags) const
        {
            return (objectFlags & flags) == flags;
        }

        INLINE bool HasAnyObjectFlags(ObjectFlags flags) const
        {
            return (objectFlags & flags) != 0;
        }

        INLINE void EnableObjectFlags(ObjectFlags flags)
        {
            objectFlags = objectFlags | flags;
        }

        INLINE void DisableObjectFlags(ObjectFlags flags)
        {
            objectFlags = objectFlags & ~flags;
        }

        INLINE bool IsDefaultInstance() const
        {
            return HasAnyObjectFlags(OF_ClassDefaultInstance);
        }

        INLINE bool IsTemplate() const
        {
            return HasAnyObjectFlags(OF_TemplateInstance);
        }

        bool IsPendingDestruction() const
        {
            return HasAnyObjectFlags(OF_PendingDestroy);
        }

		bool IsTransient() const;

		inline u32 GetSubObjectCount() const { return attachedObjects.GetObjectCount(); }
        inline Object* GetSubObject(int index) const { return attachedObjects.GetObjectAt(index); }
		inline const ObjectMap& GetSubObjectMap() const { return attachedObjects; }

    	bool HasSubObject(Ref<Object> subObject) const { return attachedObjects.ObjectExists(subObject); }
    	bool HasSubObject(Uuid subObjectUuid) const { return attachedObjects.ObjectExists(subObjectUuid); }

        // Lifecycle

        void AddToRoot();
        void RemoveFromRoot();

        virtual void AttachSubobject(Object* subobject);
        
        virtual void DetachSubobject(Object* subobject);

		virtual bool HasSubobject(Object* subobject);

		bool IsObjectPresentInHierarchy(Object* searchObject);

        void BeginDestroy();

        // - Public API -

        virtual bool IsAsset() const { return false; }

        bool IsBundle() const;

		bool IsOfType(ClassType* classType) const;

		template<typename T>
		FORCE_INLINE bool IsOfType() const
		{
			return IsOfType(T::StaticType());
		}

		template<typename TClass> requires TIsBaseClassOf<CE::Object, TClass>::Value
		static TClass* CastTo(Object* instance)
		{
			if (instance == nullptr || !instance->IsOfType<TClass>())
			{
				return nullptr;
			}
			return (TClass*)instance;
		}

    	template<typename TClass> requires TIsBaseClassOf<CE::Object, TClass>::Value
		static Ref<TClass> CastTo(Ref<Object> instance)
		{
			if (instance == nullptr || !instance->IsOfType<TClass>())
			{
				return nullptr;
			}
			return (TClass*)instance.Get();
		}

        virtual u64 ComputeMemoryFootprint();

		Name GetPathInBundle();
        
        Name GetPathInBundle(Bundle* bundle);
        
		// Returns the bundle this object belongs to.
        Bundle* GetBundle();

		// Internal use only! Returns a list of all objects that this object and it's subobjects reference to.
		void FetchObjectReferences(HashMap<Uuid, Object*>& outReferences);

        void FetchSubObjectsRecursive(Array<Object*>& outSubObjects);

		void LoadFromTemplate(Object* templateObject);

        // - Config API -

        void LoadConfig(ClassType* configClass, String fileName);

        // - Updates -

        virtual void OnFieldChanged(const Name& fieldName);

        virtual void OnFieldEdited(const Name& fieldName);

        virtual void OnFieldValidate() {}

    protected:

		void LoadFromTemplateHelper(HashMap<Uuid, Object*>& originalToClonedObjectMap, Object* templateObject);

		void LoadFromTemplateFieldHelper(HashMap<Uuid, Object*>& originalToClonedObjectMap,
            const Ptr<FieldType>& srcField, void* srcInstance, const Ptr<FieldType>& dstField, void* dstInstance);

		// Lifecycle

		virtual void OnAfterConstruct() {}
		virtual void OnBeginDestroy() {}
        virtual void OnBeforeDestroy() {}

		virtual void OnSubobjectAttached(Object* subobject) {}

		virtual void OnSubobjectDetached(Object* subobject) {}

		virtual void OnAfterDeserialize() {}
		virtual void OnBeforeSerialize() {}

		// Protected API

		Object* CreateDefaultSubobject(ClassType* classType, const String& name, ObjectFlags flags = OF_NoFlags);

		template<typename TClass> requires TIsBaseClassOf<Object, TClass>::Value
		TClass* CreateDefaultSubobject(const String& name, ObjectFlags flags = OF_NoFlags)
		{
			return (TClass*)CreateDefaultSubobject(TClass::Type(), name, flags | OF_DefaultSubobject);
		}

		template<typename TClass> requires TIsBaseClassOf<Object, TClass>::Value
		TClass* CreateDefaultSubobject(ClassType* classType, const String& name, ObjectFlags flags = OF_NoFlags)
		{
			if (!classType->IsSubclassOf<TClass>())
				return nullptr;
			return (TClass*)CreateDefaultSubobject(classType, name, flags | OF_DefaultSubobject);
		}
		
		void LoadDefaults();

        void ConfigParseStruct(const String& value, void* instance, StructType* structType);
        
        void ConfigParseField(const String& value, void* instance, const Ptr<FieldType>& field);
        
        virtual void OnAfterConfigLoad() {}


	private:

        void UnbindAllEvents();

        void OnAfterConstructInternal();

		Object* GetDefaultSubobject(ClassType* classType, const String& name);

		void FetchObjectReferencesInStructField(HashMap<Uuid, Object*>& outReferences, StructType* structType, void* structInstance);

        void DestroyImmediate();

#if PAL_TRAIT_BUILD_TESTS
		friend class ::Bundle_WriteRead_Test;
#endif
        
        friend class Bundle;
		friend class SaveBundleContext;
        friend class BinaryDeserializer;
        friend class FieldType;

        friend Object* Internal::CreateObjectInternal(const Internal::ObjectCreateParams& params);

        template<typename T>
        friend struct Internal::TypeInfoImpl;

        template<typename TObject>
        friend class Ref;

        template<typename TObject>
        friend class WeakRef;

        friend class Internal::RefCountControl;
        
        /*
         *  Fields
         */

        Name name;
        Uuid uuid;

        // Subobject Lifecycle
		ObjectMap attachedObjects{};
        
        WeakRef<Object> outer = nullptr;
        Internal::RefCountControl* control = nullptr;

        ObjectFlags objectFlags = OF_NoFlags;


    };
    
} // namespace CE

CE_RTTI_CLASS(CORE_API, CE, Object,
    CE_SUPER(),
    CE_NOT_ABSTRACT,
    CE_ATTRIBS(),
    CE_FIELD_LIST(
        CE_FIELD(name, Hidden, ReadOnly, Internal) // name cannot be modified directly
        CE_FIELD(uuid, Hidden, ReadOnly, Internal) // uuid cannot be modified directly
		CE_FIELD(attachedObjects, Hidden, ReadOnly)
		CE_FIELD(outer, Hidden, ReadOnly)
    ),
    CE_FUNCTION_LIST(
        
    )
)
