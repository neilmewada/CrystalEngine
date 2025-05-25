#pragma once

#include "RTTI/RTTIDefines.h"
#include "RTTI/RTTI.h"

#include "Field.h"
#include "Function.h"

#include "Containers/Array.h"
#include "ClassMacros.h"

#include <iostream>
#include <utility>

namespace CE
{
	// Forward Decls

	class StructType;
	class ClassType;
	class Variant;
	class Object;

	class BinaryBlob;

	template<typename ElementType>
	class Array;

	namespace Internal
	{
		template<typename Struct>
		struct TypeInfoImpl;
	}

	// *************************************************
	// RTTI

	// Default implementation always returns nullptr. Specialization will return the correct data
	template<typename Type>
	StructType* GetStaticStruct()
	{
		return nullptr;
	}

	// Default implementation always returns nullptr. Specialization will return the correct data
	template<typename Type>
	ClassType* GetStaticClass()
	{
		return nullptr;
	}

	// *************************************************
	// Struct Type

	namespace Internal
	{
		class CORE_API IStructTypeImpl
		{
		public:
			virtual void InitializeDefaults(void* instance) const = 0;
			virtual void CallDestructor(void* instance) const = 0;

			virtual Variant CreateVariant(void* instance) const = 0;
			virtual void LoadFromVariant(const Variant& srcVariant, void* dstInstance) const = 0;

			virtual void CopyConstructor(void* srcInstance, void* dstInstance) = 0;

			virtual const CE::Name& GetTypeName() const = 0;

			virtual String GetStructBundle() const = 0;
			virtual String GetStructModule() const = 0;

			virtual void OnAfterDeserialize(void* instance) = 0;
			virtual void OnBeforeSerialize(void* instance) = 0;
		};
	}

	class CORE_API StructType : public TypeInfo
	{
	protected:
		StructType(String name, Internal::IStructTypeImpl* impl, u32 size, String attributes = "") : TypeInfo(name, attributes), Impl(impl), size(size)
		{}
		virtual ~StructType();

		template<typename T>
		friend TypeInfo* GetStaticType();

		template<typename Struct>
		friend struct CE::Internal::TypeInfoImpl;

	public:

		//StructType(const StructType& copy);

		// TypeData is always located AFTER TypeInfo in memory
		virtual const u8* GetRawTypeData() const { return (u8*)(this + 1); }

		const CE::Name& GetTypeName() override { return Impl->GetTypeName(); }

		virtual TypeId GetTypeId() const override
		{
			return *(TypeId*)(GetRawTypeData() + sizeof(TypeId));
		}

		virtual bool IsStruct() const override { return true; }

		virtual bool IsAssignableTo(TypeId typeId) override;

        const Attribute& GetAttributes() override;

        bool HasAttribute(const CE::Name& key) override;

		Attribute& GetAttribute(const CE::Name& key) override;

		IntPtr TryCast(IntPtr ptr, TypeId castToType) const
		{
			const u8* data = GetRawTypeData();
			SIZE_T byteIndex = 0;
			PtrDiff offset = 0;

			while (true)
			{
				TypeIdSize size = *(TypeIdSize*)(data + byteIndex);
				byteIndex += sizeof(TypeIdSize);

				for (TypeIdSize i = 0; i < size; i++, byteIndex += sizeof(TypeIdSize))
				{
					if (*(TypeIdSize*)(data + byteIndex) == castToType)
						return ptr + offset;
				}

				offset = *(PtrDiff*)(data + byteIndex);

				if (offset == 0)
					return 0;

				byteIndex += sizeof(PtrDiff);
			}

			return 0;
		}

		u32 GetLocalFieldCount() const
		{
			return localFields.GetSize();
		}

		Ptr<FieldType> GetLocalFieldAt(u32 index)
		{
			return localFields[index];
		}

		u32 GetFunctionCount()
		{
			if (!functionsCached)
			{
				CacheAllFunctions();
			}
			return cachedFunctions.GetSize();
		}

		FunctionType* GetFunctionAt(u32 index)
		{
			if (!functionsCached)
			{
				CacheAllFunctions();
			}
			return &cachedFunctions[index];
		}

		Array<FunctionType*> GetAllFunctions();
		Array<FunctionType*> GetAllSignals();

		u32 GetLocalFunctionCount() const
		{
			return localFunctions.GetSize();
		}

		FunctionType* GetLocalFunctionAt(u32 index)
		{
			return &localFunctions[index];
		}

		u32 GetSuperTypesCount() const
		{
			return superTypeIds.GetSize();
		}

		TypeId GetSuperType(u32 index) const
		{
			return superTypeIds[index];
		}

		bool HasEventFields();

		Ptr<FieldType> GetFirstField();

		// Returns a list of all fields that are an Object Pointer
		Array<FieldType*> FetchObjectFields();

		u32 GetFieldCount();
		Ptr<FieldType> GetFieldAt(u32 index);
        
        Ptr<FieldType> FindField(const Name& name);

		bool FindFieldInstanceRelative(const Name& relativeInstancePath, const Ref<Object>& targetObject, 
			Ptr<FieldType>& outField, void*& outInstance);

		bool FindFieldInstanceRelative(const Name& relativeInstancePath, const Ref<Object>& targetObject,
			Ptr<FieldType>& outField, Ref<Object>& outObject, void*& outInstance);

		bool FindFieldInstanceRelative(const Name& relativeInstancePath, const Ref<Object>& targetObject, void* targetInstance,
			StructType*& outFieldOwner, Ptr<FieldType>& outField, Ref<Object>& outObject, void*& outInstance);

		bool HasFunctions();

		FunctionType* FindFunctionWithName(const Name& name);
		CE::Array<FunctionType*> FindAllFunctions(const Name& name);

		template<typename ReturnType, typename ClassOrStruct, typename... Args>
		FunctionType* FindFunction(const Name& name, ReturnType(ClassOrStruct::* function)(Args...))
		{
			auto functions = FindAllFunctions(name);
			
			TypeId signature = CE::GetFunctionSignature(function);

			for (auto funcType : functions)
			{
				if (funcType->GetFunctionSignature() == signature && funcType->GetName() == name)
				{
					return funcType;
				}
			}

			return nullptr;
		}

		template<typename ReturnType, typename ClassOrStruct, typename... Args>
		FunctionType* FindFunction(const Name& name, ReturnType(ClassOrStruct::* function)(Args...) const)
		{
			auto functions = FindAllFunctions(name);

			TypeId signature = CE::GetFunctionSignature(function);

			for (auto funcType : functions)
			{
				if (funcType->GetFunctionSignature() == signature && funcType->GetName() == name)
				{
					return funcType;
				}
			}

			return nullptr;
		}

		virtual void InitializeDefaults(void* instance) override
		{
			if (Impl == nullptr || instance == nullptr)
				return;
			return Impl->InitializeDefaults(instance);
		}

		virtual void CallDestructor(void* instance) override
		{
			if (Impl == nullptr || instance == nullptr)
				return;
			return Impl->CallDestructor(instance);
		}

		virtual Variant CreateVariant(void* instance)
		{
			if (Impl == nullptr || instance == nullptr)
				return Variant();
			return Impl->CreateVariant(instance);
		}

		virtual void LoadFromVariant(const Variant& srcVariant, void* dstInstance)
		{
			if (Impl == nullptr || dstInstance == nullptr)
				return;
			Impl->LoadFromVariant(srcVariant, dstInstance);
		}

		void CopyConstructor(void* source, void* destination) override
		{
			if (Impl == nullptr || source == nullptr || destination == nullptr)
				return;
			Impl->CopyConstructor(source, destination);
		}

		/// To be called only on struct types
		void OnAfterDeserialize(void* instance)
		{
			if (Impl != nullptr)
				Impl->OnAfterDeserialize(instance);
		}

		/// To be called only on struct types
		void OnBeforeSerialize(void* instance)
		{
			if (Impl != nullptr)
			{
				Impl->OnBeforeSerialize(instance);
			}
		}

		String GetStructBundle() const
		{
			if (Impl == nullptr)
				return nullptr;
			return Impl->GetStructBundle();
		}

		String GetStructModule() const
		{
			if (Impl == nullptr)
				return nullptr;
			return Impl->GetStructModule();
		}

		virtual u32 GetSize() const override
		{
			return size;
		}

        // For internal use only!
        static void RegisterStructType(StructType* type);
        // For internal use only!
        static void DeregisterStructType(StructType* type);

        static StructType* FindStructByName(Name structName);
        static StructType* FindStructById(TypeId structTypeId);
        
        inline static StructType* FindStruct(Name className)
        {
            return FindStructByName(className);
        }

        inline static StructType* FindStruct(TypeId classId)
        {
            return FindStructById(classId);
        }

	protected:

        virtual void CacheAllAttributes();

		void MergeAttributes(const Attribute& attribs, bool inherit = false);

		virtual void CacheAllFields();

		virtual void CacheAllFunctions();

		template<typename StructOrClass, typename Field>
		void AddField(const char* name, Field StructOrClass::* field, SIZE_T offset, const char* attributes, TypeId underlyingTypeId = 0)
		{
			using RefCounted = TRefCounted<Field>;

			constexpr bool isPointer = std::is_pointer_v<Field>;
			constexpr bool isRef = TIsRef<Field>::Value;
			constexpr bool isWeakRef = TIsWeakRef<Field>::Value;
			typedef RemovePointerFromType<Field> _Type0;
			typedef RemoveReferenceFromType<_Type0> _Type1;
			typedef RemoveConstVolatileFromType<_Type1> FinalType;
			constexpr bool isObject = TIsBaseClassOf<CE::Object, FinalType>::Value;
			constexpr bool isTypeInfo = TIsBaseClassOf<CE::TypeInfo, FinalType>::Value;

			static_assert(!isPointer || isObject || isTypeInfo, "Pointer types should only be used for object fields!");

			RefType refType = RefCounted::GetRefType();

			localFields.Add(new FieldType(name,
                                      CE::GetTypeId<Field>(),
                                      underlyingTypeId,
                                      sizeof(Field), offset, attributes, this, refType));
		}

	private:

		template<typename ReturnType, typename ClassOrStruct, typename... Args, std::size_t... Is>
		void AddFunction(const char* name, ReturnType(ClassOrStruct::* function)(Args...), const char* attributes, std::index_sequence<Is...>)
		{
			ReturnType(ClassOrStruct::*funcPtr)(Args...) = function;

			static_assert(!TIsTypePresent<CE::Variant, Args...>::Value, "A reflected function should NOT have a parameter of CE::Variant type.");

			FunctionDelegate funcDelegate = [funcPtr](void* instance, const Array<Variant>& params, CE::Variant& returnValue) -> void
			{
				if constexpr (std::is_same_v<ReturnType, void>) // No return value
				{
					(((ClassOrStruct*)instance)->*funcPtr)( ((params.begin() + Is)->GetValue<Args>())... );
					returnValue = CE::Variant();
				}
				else
				{
					auto value = (((ClassOrStruct*)instance)->*funcPtr)( ((params.begin() + Is)->GetValue<Args>())... );
					returnValue = CE::Variant(value);
				}
			};

			FunctionType funcType = FunctionType(name, CE::GetTypeId<ReturnType>(), std::initializer_list<TypeId>{ CE::GetTypeId<Args>()... }, 
				funcDelegate, this, attributes,
				TGetUnderlyingTypeId<ReturnType>::Get(),
				TGetUnderlyingTypeIdPack<Args...>::Get());

			localFunctions.Add(funcType);
		}

		template<typename ReturnType, typename ClassOrStruct, typename... Args, std::size_t... Is>
		void AddFunction(const char* name, ReturnType(ClassOrStruct::* function)(Args...) const, const char* attributes, std::index_sequence<Is...>)
		{
			ReturnType(ClassOrStruct::*funcPtr)(Args...) const = function;

			static_assert(!TIsTypePresent<CE::Variant, Args...>::Value, "A reflected function should NOT have a parameter of CE::Variant type.");

			FunctionDelegate funcDelegate = [funcPtr](void* instance, const Array<Variant>& params, CE::Variant& returnValue) -> void
			{
				if constexpr (std::is_same_v<ReturnType, void>) // No return value
				{
					(((ClassOrStruct*)instance)->*funcPtr)( ((params.begin() + Is)->GetValue<Args>())... );
					returnValue = CE::Variant();
				}
				else
				{
					auto value = (((ClassOrStruct*)instance)->*funcPtr)( ((params.begin() + Is)->GetValue<Args>())... );
					returnValue = CE::Variant(value);
				}
			};

			localFunctions.Add(FunctionType(name, CE::GetTypeId<ReturnType>(), { CE::GetTypeId<Args>()... }, funcDelegate, this, String(attributes) + ",Constant",
				TGetUnderlyingTypeId<ReturnType>::Get(),
				TGetUnderlyingTypeIdPack<Args...>::Get()));
		}

	protected:

		template<typename ReturnType, typename ClassOrStruct, typename... Args>
		void AddFunction(const char* name, ReturnType (ClassOrStruct::* function)(Args...), const char* attributes)
		{
			AddFunction<ReturnType, ClassOrStruct, Args...>(name, function, attributes, std::make_index_sequence<sizeof...(Args)>());
		}

		template<typename ReturnType, typename ClassOrStruct, typename... Args>
		void AddFunction(const char* name, ReturnType(ClassOrStruct::* function)(Args...) const, const char* attributes)
		{
			AddFunction<ReturnType, ClassOrStruct, Args...>(name, function, attributes, std::make_index_sequence<sizeof...(Args)>());
		}

		template<typename... SuperTypes>
		void AddSuper()
		{
			TypeId ids[] = { CE::GetTypeId<SuperTypes>()... };
			constexpr int count = sizeof(ids) / sizeof(TypeId);
			for (int i = 0; i < count; i++)
			{
				superTypeIds.Add(ids[i]);
			}
		}

		template<>
		void AddSuper<>() {}

		// Inherited + Local fields
		SharedMutex cachedFieldsMutex{};
		CE::Array<Ptr<FieldType>> cachedFields{};
		CE::HashMap<CE::Name, Ptr<FieldType>> cachedFieldsMap{};
        
		SharedMutex cachedFunctionsMutex{};
		CE::Array<FunctionType> cachedFunctions{};
        CE::HashMap<CE::Name, Array<FunctionType*>> cachedFunctionsMap{};

		SharedMutex cachedAttributesMutex{};
        Attribute cachedAttributes{};

		CE::Array<Ptr<FieldType>> localFields{};
		CE::Array<TypeId> superTypeIds{};
		CE::Array<FunctionType> localFunctions{};
		Atomic<bool> fieldsCached = false;
		Atomic<bool> functionsCached = false;
		Atomic<bool> attributesCached = false;
		u32 size = 0;
		bool hasEventFields = false;

		//RecursiveMutex rttiMutex{};

        static CE::HashMap<TypeId, StructType*> registeredStructs;
        static CE::HashMap<Name, StructType*> registeredStructsByName;

		friend class ClassType;

	private:
		Internal::IStructTypeImpl* Impl = nullptr;
	};

	typedef StructType Struct;


	// *************************************************
	// Class Type

	namespace Internal
	{
		class CORE_API IClassTypeImpl
		{
		public:
			virtual Object* CreateInstance() const = 0;
			
			virtual void InitializeDefaults(void* instance) const = 0;
			virtual void CallDestructor(void* instance) const = 0;

			virtual bool CanInstantiate() const = 0;

			virtual const CE::Name& GetTypeName() const = 0;

			virtual String GetClassBundle() const = 0;

			virtual String GetClassModule() const = 0;
		};
	}

	class CORE_API ClassType : public StructType
	{
	protected:
		ClassType(String name, Internal::IClassTypeImpl* impl, u32 size, String attributes = "");

		template<typename T>
		friend TypeInfo* GetStaticType();

		template<typename Class>
		friend struct CE::Internal::TypeInfoImpl;

	private:
		void ConstructInternal();

	public:

		const CE::Name& GetTypeName() override { return Impl->GetTypeName(); }

		// TypeData is always located AFTER TypeInfo in memory
		virtual const u8* GetRawTypeData() const override { return (u8*)(this + 1); }

		virtual TypeId GetTypeId() const override
		{
			return *(TypeId*)(GetRawTypeData() + sizeof(TypeId));
		}

		virtual bool IsStruct() const override { return false; }
		virtual bool IsClass() const override { return true; }

		bool IsSubclassOf(TypeId baseClassId);

		bool IsSubclassOf(ClassType* baseClass)
		{
			if (baseClass == nullptr)
				return false;
			return IsSubclassOf(baseClass->GetTypeId());
		}

		template<typename TBaseClass>
		inline bool IsSubclassOf()
		{
			return IsSubclassOf(TBaseClass::Type());
		}

		virtual bool IsObject();

		virtual Object* CreateInstance() const
		{
			if (Impl == nullptr)
				return nullptr;
			return Impl->CreateInstance();
		}

		virtual bool CanBeInstantiated() const
		{
			if (Impl == nullptr)
				return false;
			return Impl->CanInstantiate();
		}

		virtual void InitializeDefaults(void* instance) override
		{
			// TODO: Reimplement this. Load CDI instead of calling default constructor.
			if (Impl == nullptr || instance == nullptr)
				return;
			return Impl->InitializeDefaults(instance);
		}

		virtual void CallDestructor(void* instance) override
		{
			if (Impl == nullptr || instance == nullptr)
				return;
			return Impl->CallDestructor(instance);
		}

		String GetClassBundle() const
		{
			if (Impl == nullptr)
				return "";
			return Impl->GetClassBundle();
		}

		String GetClassModule() const
		{
			if (Impl == nullptr)
				return "";
			return Impl->GetClassModule();
		}

		const Object* GetDefaultInstance();

        // For internal use only!
        static void RegisterClassType(ClassType* type);
        // For internal use only!
        static void DeregisterClassType(ClassType* type);

		static void CreateDefaultInstancesForCurrentModule();
		static void CacheTypesForCurrentModule();

		static bool GetTotalRegisteredClasses()
		{
			return registeredClasses.GetSize();
		}

        static ClassType* FindClassByName(const Name& className);
        static ClassType* FindClassById(TypeId classId);

		inline static ClassType* FindClass(const Name& className)
		{
			return FindClassByName(className);
		}

		inline static ClassType* FindClass(TypeId classId)
		{
			return FindClassById(classId);
		}

		ClassType* GetSuperClass(u32 index)
		{
			if (!superTypesCached)
				CacheSuperTypes();
			return (index >= 0 && index < superClasses.GetSize()) ? superClasses[index] : nullptr;
		}

		u32 GetSuperClassCount()
		{
			if (!superTypesCached)
				CacheSuperTypes();
			return superClasses.GetSize();
		}

        Array<TypeId> GetDerivedClassesTypeId() const;
        Array<ClassType*> GetDerivedClasses() const;

	private:

		static void ClearDefaultInstancesForModule(const Name& moduleName);

        static void AddDerivedClassToMap(ClassType* derivedClass, ClassType* parentSearchPath);

		void CacheSuperTypes();

		Internal::IClassTypeImpl* Impl = nullptr;

		Ref<Object> defaultInstance = nullptr;

		bool superTypesCached = false;
		Array<ClassType*> superClasses{};

        static CE::HashMap<TypeId, ClassType*> registeredClasses;
        static CE::HashMap<Name, ClassType*> registeredClassesByName;

        static CE::HashMap<TypeId, Array<TypeId>> derivedClassesMap;

		friend class ModuleManager;
	};

#pragma pack(push, 1)

	typedef ClassType Class;

	namespace Internal
	{
		// Specialization will contain the required data
		template<typename T>
		struct TypeInfoImpl
		{

		};

		// Recursively populates the StructTypeData
		// Layout of typeData:
		// [ TypeIdSize size, TypeId firstTypeId ... TypeId lastTypeId, PtrDiff offset/endMarker if = 0,
		// TypeIdSize size, TypeId firstTypeId ... TypeId lastTypeId, PtrDiff offset/endMarker if = 0... ]
		// Each block represents inherited types from a base, the first block doesn't need offset as it is implicitly 0
		// Therefore we can use the offset as an end marker, all other bases will have a positive offset
		template<typename... BaseTypes>
		struct BaseStructTypeData
		{};

		// Specialization of struct BaseStructTypeData<BaseTypes...>;
		template<typename Base1, typename Base2, typename... BaseN>
		struct BaseStructTypeData<Base1, Base2, BaseN...>
		{
			template<typename Derived>
			void FillBaseStructTypeData(PtrDiff inOffset, TypeIdSize& outHeadSize)
			{
				BaseStructTypeData1.template FillBaseStructTypeData<Derived>(ComputePointerOffset<Derived, Base1>(), outHeadSize);

				Offset = ComputePointerOffset<Derived, Base2>();
				BaseStructTypeDataRemaining.template FillBaseStructTypeData<Derived>(Offset, Size);
			}

			BaseStructTypeData<Base1> BaseStructTypeData1;
			PtrDiff Offset;
			TypeIdSize Size;
			BaseStructTypeData<Base2, BaseN...> BaseStructTypeDataRemaining;
		};

		template<typename Base>
		struct BaseStructTypeData<Base>
		{
			template<typename Derived>
			void FillBaseStructTypeData(PtrDiff inOffset, TypeIdSize& outHeadSize)
			{
				StructTypeData<Base>* baseTypeData = nullptr;
				if (GetStaticStruct<Base>() != nullptr)
				{
					baseTypeData = (StructTypeData<Base>*)(GetStaticStruct<Base>()->GetRawTypeData());
				}
				else if (GetStaticClass<Base>() != nullptr)
				{
					baseTypeData = (StructTypeData<Base>*)(GetStaticClass<Base>()->GetRawTypeData());
				}
				else
				{
					throw std::runtime_error(std::string("Failed to find struct or class ") + typeid(Base).name());
					return;
				}

				// return size of head list
				outHeadSize = baseTypeData->Size;

				const char* data = baseTypeData->GetData();
				SIZE_T byteSize = baseTypeData->Size * sizeof(TypeIdSize);

				// copy type list
				memcpy(Data, data, byteSize);

				SIZE_T byteIndex = byteSize;
				PtrDiff offset = *(PtrDiff*)(data + byteIndex);

				while (offset != 0)
				{
					// fill next offset and add pointer offset
					*(PtrDiff*)(Data + byteIndex) = offset + inOffset;
					byteIndex += sizeof(PtrDiff);

					// fill next size
					const TypeIdSize size = *(TypeIdSize*)(data + byteIndex);
					*(TypeIdSize*)(Data + byteIndex) = size;

					byteSize = size * sizeof(TypeIdSize);
					byteIndex += sizeof(TypeIdSize);

					// copy types
					memcpy(Data + byteIndex, data + byteIndex, byteSize);
					byteIndex += byteSize;
					offset = *(PtrDiff*)(data + byteIndex);
				}
			}

			// We only need the previous type data array, but not its size or end marker as we will insert them ourselves
			char Data[sizeof(StructTypeData<Base>) - sizeof(PtrDiff) - sizeof(TypeIdSize)];
		};

		// Main template for TypeDataImpl
		template<typename Type, typename... BaseTypes>
		struct TypeDataImpl
		{
			TypeDataImpl()
			{
				this->ThisTypeId = CE::GetTypeId<Type>();
				BaseStructTypeData.template FillBaseStructTypeData<Type>(0, Size);
				Size++;
				EndMarker = 0;
			}

			const char* GetData() const { return (char*)&ThisTypeId; }

			TypeId GetTypeId() const { return ThisTypeId; }

			TypeIdSize Size;
			TypeId ThisTypeId;
			BaseStructTypeData<BaseTypes...> BaseStructTypeData;
			PtrDiff EndMarker;
		};

		// Specialization of TypeDataImpl<Type, BaseTypes...> for a type with no base types
		template<typename Type>
		struct TypeDataImpl<Type>
		{
			TypeDataImpl() : Size(1), EndMarker(0)
			{
				this->ThisTypeId = GetTypeId<Type>();
			}

			const char* GetData() const { return (char*)&ThisTypeId; }

			TypeIdSize Size;
			TypeId ThisTypeId;
			PtrDiff EndMarker;
		};
	} // namespace Internal

#pragma pack(pop)

	/*
	*	Utils
	*/

	template<typename TClass> requires TIsBaseClassOf<Object, TClass>::Value
	const TClass* GetDefaults()
	{
		ClassType* classType = TClass::Type();
		if (classType == nullptr)
			return nullptr;
		return (TClass*)classType->GetDefaultInstance();
	}

	template<typename TClass> requires TIsBaseClassOf<Object, TClass>::Value
	TClass* GetMutableDefaults()
	{
		ClassType* classType = TClass::StaticType();
		if (classType == nullptr)
			return nullptr;
		return (TClass*)classType->GetDefaultInstance();
	}

	/*
	 *	Casting
	 */

	template<typename TCastTo, typename TCastFrom>
    TCastTo* DynamicCast(TCastFrom* from)
	{
		if constexpr (std::is_void_v<TCastFrom>)
		{
			return static_cast<TCastTo*>(from);
		}
		StructType* castFrom = TCastFrom::Type();
		StructType* castTo = TCastTo::Type();
		return (TCastTo*)castFrom->TryCast((SIZE_T)from, castTo->GetTypeId());
	}

}

CE_RTTI_TYPEINFO(CORE_API, CE, StructType, TYPEID(CE::TypeInfo))
CE_RTTI_TYPEINFO(CORE_API, CE, ClassType, TYPEID(CE::TypeInfo))

/// fmt user-defined Formatter for CE::String
template <> struct fmt::formatter<CE::StructType*> {
	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		// Return an iterator past the end of the parsed range:
		return ctx.end();
	}

	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template <typename FormatContext>
	auto format(CE::StructType* structType, FormatContext& ctx) const -> decltype(ctx.out()) {
		// ctx.out() is an output iterator to write to.
		return fmt::format_to(ctx.out(), "{}", structType->GetTypeName().GetString());
	}
};

/// fmt user-defined Formatter for CE::String
template <> struct fmt::formatter<CE::ClassType*> {
	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		// Return an iterator past the end of the parsed range:
		return ctx.end();
	}

	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template <typename FormatContext>
	auto format(CE::ClassType* clazz, FormatContext& ctx) const -> decltype(ctx.out()) {
		// ctx.out() is an output iterator to write to.
		return fmt::format_to(ctx.out(), "{}", clazz->GetTypeName().GetString());
	}
};
