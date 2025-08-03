#pragma once


#define __CE_SUPER_LIST(...) CE_EXPAND(CE_CONCATENATE(__CE_SUPER_LIST_,CE_ARG_COUNT(__VA_ARGS__)))(__VA_ARGS__)

// - Generated body macros -

#define __CE_GENERATED_BODY_IMPL_1(Name)
#define __CE_GENERATED_BODY_IMPL_0(Name) CE_EXPAND(CE_CONCATENATE(__CE_GENERATED_BODY_, Name))

#define __CE_GENERATED_BODY_2(Name) CE_EXPAND(CE_ARG_COUNT(__CE_HAS_GENERATED_BODY_##Name))

#define CE_GENERATED_BODY(Name) CE_EXPAND(CE_CONCATENATE(__CE_GENERATED_BODY_IMPL_, __CE_GENERATED_BODY_2(Name)))(Name)

// - Field and Function macros -

#define CE_FIELD_LIST(x) x
#define CE_FIELD(FieldName, ...)\
	Type.AddField(#FieldName, &Self::FieldName, offsetof(Self, FieldName), "" #__VA_ARGS__, TYPEID(TGetUnderlyingType<decltype(Self::FieldName)>::Type));

#define CE_PROPERTY(PropertyName, VariableName) \
	const decltype(VariableName)& Get##PropertyName() const { return VariableName; }\
	void Set##PropertyName(const decltype(VariableName)& value)\
	{ \
		VariableName = value; \
		thread_local CE::Name fieldName = #VariableName; \
		this->OnFieldChanged(fieldName); \
	}

#define CE_FUNCTION_LIST(x) x
#define CE_FUNCTION(FunctionName, ...) Type.AddFunction(#FunctionName, &Self::FunctionName, "" #__VA_ARGS__);

#define CE_FUNCTION2(FunctionName, ReturnType, Signature, ...)\
{\
	ReturnType (Self::*funcPtr)Signature = &Self::FunctionName;\
	Type.AddFunction(#FunctionName, funcPtr, "" #__VA_ARGS__);\
}

#define MEMBER_FUNCTION(ClassOrStruct, FunctionName) ClassOrStruct::Type()->FindFunction(#FunctionName, &ClassOrStruct::FunctionName)

#define __CE_RTTI_JOIN_CLASSES_0()
#define __CE_RTTI_JOIN_CLASSES_1(...) , __VA_ARGS__
#define __CE_RTTI_JOIN_CLASSES_2(...) , __VA_ARGS__
#define __CE_RTTI_JOIN_CLASSES_3(...) , __VA_ARGS__
#define __CE_RTTI_JOIN_CLASSES_4(...) , __VA_ARGS__
#define __CE_RTTI_JOIN_CLASSES_5(...) , __VA_ARGS__
#define __CE_RTTI_JOIN_CLASSES_6(...) , __VA_ARGS__
#define __CE_RTTI_JOIN_CLASSES_7(...) , __VA_ARGS__
#define __CE_RTTI_JOIN_CLASSES_8(...) , __VA_ARGS__
#define __CE_RTTI_JOIN_CLASSES_9(...) , __VA_ARGS__

#define __CE_RTTI_JOIN_CLASSES(Class, ...) Class CE_EXPAND(CE_CONCATENATE(__CE_RTTI_JOIN_CLASSES_,CE_ARG_COUNT(__VA_ARGS__)))(__VA_ARGS__)

#define CE_SUPER(...) __VA_ARGS__
#define CE_ATTRIBS(...) #__VA_ARGS__

#define __CE_NEW_INSTANCE_(Namespace, Class) new Namespace::Class
#define __CE_NEW_INSTANCE_NotAbstract(Namespace, Class) new Namespace::Class
#define __CE_NEW_INSTANCE_false(Namespace, Class) new Namespace::Class
#define __CE_NEW_INSTANCE_Abstract(Namespace, Class) nullptr
#define __CE_NEW_INSTANCE_true(Namespace, Class) nullptr

#define __CE_CAN_INSTANTIATE_() true
#define __CE_CAN_INSTANTIATE_Abstract() false
#define __CE_CAN_INSTANTIATE_NotAbstract() true
#define __CE_CAN_INSTANTIATE_false() true
#define __CE_CAN_INSTANTIATE_true() false

#define __CE_INIT_DEFAULTS_(Namespace, Class) new(instance) Namespace::Class
#define __CE_INIT_DEFAULTS_NotAbstract(Namespace, Class) new(instance) Namespace::Class
#define __CE_INIT_DEFAULTS_false(Namespace, Class) new(instance) Namespace::Class
#define __CE_INIT_DEFAULTS_Abstract(Namespace, Class)
#define __CE_INIT_DEFAULTS_true(Namespace, Class)

#define CE_ABSTRACT Abstract
#define CE_NOT_ABSTRACT NotAbstract
#define CE_DONT_INSTANTIATE Abstract
#define CE_INSTANTIATE NotAbstract

#define __CE_USING_NAMESPACE_IMPL_0()
#define __CE_USING_NAMESPACE_IMPL_1(...) using namespace __VA_ARGS__;

#define CE_USING_NAMESPACE(Namespace) CE_EXPAND(CE_CONCATENATE(__CE_USING_NAMESPACE_IMPL_,CE_ARG_COUNT(Namespace)))(Namespace)

#define __MERGE_NAMESPACE_IMPL_0(Namespace, SubNamespace) SubNamespace
#define __MERGE_NAMESPACE_IMPL_1(Namespace, SubNamespace) Namespace::SubNamespace

// Merge Namespace and SubNamespace in a way where Namespace can possibly be empty but SubNamespace will always be valid
#define MERGE_NAMESPACE(Namespace, SubNamespace) CE_EXPAND(CE_CONCATENATE(__MERGE_NAMESPACE_IMPL_,CE_ARG_COUNT(Namespace)))(Namespace, SubNamespace)

#define CE_RTTI_CLASS(API, Namespace, Class, SuperClasses, IsAbstract, Attributes, FieldList, FunctionList)\
namespace CE\
{\
	CE_USING_NAMESPACE(Namespace);\
	template<>\
	struct StructTypeData<Namespace::Class> : public CE::Internal::TypeDataImpl<__CE_RTTI_JOIN_CLASSES(Namespace::Class, SuperClasses)>\
	{};\
	namespace Internal\
	{\
		template<>\
		struct TypeInfoImpl<Namespace::Class> : public CE::Internal::IClassTypeImpl\
		{\
            typedef Namespace::Class Self;\
            CE::ClassType Type;\
			API static const CE::Name& FullTypeName();\
			const CE::StructTypeData<Namespace::Class> TypeData;\
            TypeInfoImpl(const char* fullName, CE::Internal::IClassTypeImpl* impl, u32 size, CE::StructTypeData<Namespace::Class> typeData)\
				: Type(fullName, impl, size, Attributes "")\
				, TypeData(typeData)\
            {\
				static_assert(std::is_same_v<Namespace::Class::Self, Namespace::Class>, "Reflected class must have CE_CLASS() in body!");\
				Type.AddSuper<SuperClasses>();\
				FieldList\
				FunctionList\
            }\
			virtual ~TypeInfoImpl()\
			{\
			}\
			virtual bool CanInstantiate() const override\
			{\
				return CE_EXPAND(CE_CONCATENATE(__CE_CAN_INSTANTIATE_,CE_FIRST_ARG(IsAbstract)))();\
			}\
			virtual Object* CreateInstance() const override\
			{\
				return CE_EXPAND(CE_CONCATENATE(__CE_NEW_INSTANCE_,CE_FIRST_ARG(IsAbstract)))(Namespace, Class);\
			}\
			virtual void InitializeDefaults(void* instance) const override\
			{\
				CE_EXPAND(CE_CONCATENATE(__CE_INIT_DEFAULTS_,CE_FIRST_ARG(IsAbstract)))(Namespace, Class);\
			}\
			virtual void CallDestructor(void* instance) const override\
			{\
				((Namespace::Class*)instance)->~Class();\
			}\
			virtual const CE::Name& GetTypeName() const override { return FullTypeName(); }\
			virtual String GetClassBundle() const override\
			{\
				return Namespace::Class::StaticBundle();\
			}\
			virtual String GetClassModule() const override\
			{\
				return Namespace::Class::StaticModule();\
			}\
		};\
	}\
	template<>\
	inline TypeInfo* GetStaticType<Namespace::Class>()\
	{\
		return Namespace::Class::Type();\
	}\
	template<>\
	inline CE::Name GetTypeName<Namespace::Class>()\
	{\
		return CE::Internal::TypeInfoImpl<Namespace::Class>::FullTypeName();\
	}\
	template<>\
	inline ClassType* GetStaticClass<Namespace::Class>()\
	{\
		return (ClassType*)GetStaticType<Namespace::Class>();\
	}\
}

#define CE_RTTI_CLASS_IMPL(API, Namespace, Class)\
CE::ClassType* Namespace::Class::Type()\
{\
	static CE::Internal::TypeInfoImpl<Namespace::Class> instance{\
			#Namespace "::" #Class,\
			&instance, sizeof(Namespace::Class),\
			CE::StructTypeData<Namespace::Class>()\
	};\
	return &instance.Type;\
}\
const CE::Name& CE::Internal::TypeInfoImpl<Namespace::Class>::FullTypeName()\
{\
	static CE::Name name = MAKE_NAME(BUNDLE_NAME, Namespace, Class);\
	return name;\
}\
CE::String MERGE_NAMESPACE(Namespace, Class)::StaticBundle()\
{\
	return BUNDLE_NAME;\
}\
CE::String MERGE_NAMESPACE(Namespace, Class)::StaticModule()\
{\
	return MODULE_NAME;\
}

#define __CE_RTTI_SUPERCLASS_0(...) typedef void Super;
#define __CE_RTTI_SUPERCLASS_1(SuperClass) typedef SuperClass Super;
#define __CE_RTTI_SUPERCLASS_2(SuperClass, ...) __CE_RTTI_SUPERCLASS_1(SuperClass);
#define __CE_RTTI_SUPERCLASS_3(SuperClass, ...) __CE_RTTI_SUPERCLASS_1(SuperClass);
#define __CE_RTTI_SUPERCLASS_4(SuperClass, ...) __CE_RTTI_SUPERCLASS_1(SuperClass);
#define __CE_RTTI_SUPERCLASS_5(SuperClass, ...) __CE_RTTI_SUPERCLASS_1(SuperClass);
#define __CE_RTTI_SUPERCLASS_6(SuperClass, ...) __CE_RTTI_SUPERCLASS_1(SuperClass);
#define __CE_RTTI_SUPERCLASS_7(SuperClass, ...) __CE_RTTI_SUPERCLASS_1(SuperClass);
#define __CE_RTTI_SUPERCLASS_8(SuperClass, ...) __CE_RTTI_SUPERCLASS_1(SuperClass);
#define __CE_RTTI_SUPERCLASS_9(SuperClass, ...) __CE_RTTI_SUPERCLASS_1(SuperClass);

#define __CE_RTTI_SUPERCLASS(...) CE_MACRO_EXPAND(CE_CONCATENATE(__CE_RTTI_SUPERCLASS_, CE_ARG_COUNT(__VA_ARGS__)))(__VA_ARGS__)

#define CE_CLASS(Class, ...)\
public:\
	template<typename T>\
	friend struct CE::Internal::TypeInfoImpl;\
    typedef Class Self;\
    __CE_RTTI_SUPERCLASS(__VA_ARGS__)\
	CE_GENERATED_BODY(Class)\
    static CE::ClassType* Type();\
	inline static CE::ClassType* StaticType() { return Self::Type(); }\
	inline static CE::ClassType* StaticClass() { return Self::Type(); }\
	static CE::String StaticBundle();\
	static CE::String StaticModule();\
	constexpr static bool IsClass = true;\
	constexpr static bool IsStruct = false;\
    virtual const CE::TypeInfo* GetType() const\
    {\
        return Type();\
    }\
	virtual CE::ClassType* GetClass() const\
	{\
		return Type();\
	}\
	virtual CE::String GetClassBundle() const\
	{\
		return StaticBundle();\
	}\
	virtual CE::String GetClassModule() const\
	{\
		return StaticModule();\
	}



#define CE_RTTI_STRUCT(API, Namespace, Struct, SuperStructs, Attributes, FieldList, FunctionList)\
namespace CE\
{\
	CE_USING_NAMESPACE(Namespace);\
	template<>\
	struct StructTypeData<Namespace::Struct> : public CE::Internal::TypeDataImpl<__CE_RTTI_JOIN_CLASSES(Namespace::Struct, SuperStructs)>\
	{\
	};\
	namespace Internal\
	{\
		template<>\
		struct TypeInfoImpl<Namespace::Struct> : public CE::Internal::IStructTypeImpl\
		{\
            typedef Namespace::Struct Self;\
            CE::StructType Type;\
			const CE::StructTypeData<Namespace::Struct> TypeData;\
			API static const CE::Name& FullTypeName();\
            TypeInfoImpl(const char* name, CE::Internal::IStructTypeImpl* impl, u32 size, CE::StructTypeData<Namespace::Struct> typeData, const char* attributes = "")\
				: Type(name, impl, size, attributes), TypeData(typeData)\
            {\
				static_assert(std::is_default_constructible_v<Namespace::Struct>, "The struct " #Namespace "::" #Struct " does not have a default constructor!");\
				Type.AddSuper<SuperStructs>();\
				FieldList\
				FunctionList\
            }\
			virtual ~TypeInfoImpl()\
			{\
			}\
			virtual void InitializeDefaults(void* instance) const override\
			{\
				new(instance) Namespace::Struct;\
			}\
			virtual void CallDestructor(void* instance) const override\
			{\
				((Namespace::Struct*)instance)->~Struct();\
			}\
			virtual void CopyConstructor(void* srcInstance, void* dstInstance) override\
			{\
				new(dstInstance) Namespace::Struct(*(Namespace::Struct*)srcInstance);\
			}\
			virtual CE::Variant CreateVariant(void* instance) const override\
			{\
				return CE::Variant(*(Namespace::Struct*)instance);\
			}\
			virtual void LoadFromVariant(const Variant& srcVariant, void* dstInstance) const override\
			{\
				if (srcVariant.IsOfType<Namespace::Struct>()) { *(Namespace::Struct*)dstInstance = srcVariant.GetValue<Namespace::Struct>(); }\
			}\
			virtual const CE::Name& GetTypeName() const override { return FullTypeName(); }\
			virtual String GetStructBundle() const override\
			{\
				return Namespace::Struct::StaticBundle();\
			}\
			virtual String GetStructModule() const override\
			{\
				return Namespace::Struct::StaticModule();\
			}\
			virtual void OnAfterDeserialize(void* instance) override\
			{\
				if constexpr (THasOnAfterDeserializeFunction<Namespace::Struct>::Value)\
				{\
					THasOnAfterDeserializeFunction<Namespace::Struct>::OnAfterDeserialize((Namespace::Struct*)instance);\
				}\
			}\
			virtual void OnBeforeSerialize(void* instance) override\
			{\
				if constexpr (THasOnBeforeSerializeFunction<Namespace::Struct>::Value)\
				{\
					THasOnBeforeSerializeFunction<Namespace::Struct>::OnBeforeSerialize((Namespace::Struct*)instance);\
				}\
			}\
		};\
	}\
	template<>\
	inline TypeInfo* GetStaticType<Namespace::Struct>()\
	{\
        static CE::Internal::TypeInfoImpl<Namespace::Struct> instance{\
			#Namespace "::" #Struct, &instance, sizeof(Namespace::Struct), StructTypeData<Namespace::Struct>(), Attributes ""\
		};\
		return &instance.Type;\
	}\
	template<>\
	inline CE::Name GetTypeName<Namespace::Struct>()\
	{\
		return CE::Internal::TypeInfoImpl<Namespace::Struct>::FullTypeName();\
	}\
	template<>\
	inline StructType* GetStaticStruct<Namespace::Struct>()\
	{\
		return (StructType*)GetStaticType<Namespace::Struct>();\
	}\
}

#define CE_RTTI_STRUCT_IMPL(API, Namespace, Struct)\
CE::StructType* Namespace::Struct::Type()\
{\
	return (CE::StructType*)(CE::template GetStaticType<Self>());\
}\
const CE::Name& CE::Internal::TypeInfoImpl<Namespace::Struct>::FullTypeName()\
{\
	static CE::Name name = MAKE_NAME(BUNDLE_NAME, Namespace, Struct);\
	return name;\
}\
CE::String MERGE_NAMESPACE(Namespace, Struct)::StaticBundle()\
{\
	return BUNDLE_NAME;\
}\
CE::String MERGE_NAMESPACE(Namespace, Struct)::StaticModule()\
{\
	return MODULE_NAME;\
}

#define CE_STRUCT(Struct, ...)\
public:\
	template<typename T>\
	friend struct CE::Internal::TypeInfoImpl;\
	template<typename T, typename>\
	friend struct TStructReleaseFunction;\
    typedef Struct Self;\
    __CE_RTTI_SUPERCLASS(__VA_ARGS__)\
	template<typename T, typename>\
	friend struct CE::THasOnAfterDeserializeFunction;\
	template<typename T, typename>\
	friend struct CE::THasOnBeforeSerializeFunction;\
    static CE::StructType* Type();\
	inline static CE::StructType* StaticType() { return Self::Type(); }\
	inline static CE::StructType* StaticStruct() { return Self::Type(); }\
    constexpr static bool IsClass = false;\
    constexpr static bool IsStruct = true;\
	static CE::String StaticBundle();\
	static CE::String StaticModule();\
    virtual CE::TypeInfo* GetType() const\
    {\
        return Type();\
    }\
	virtual CE::StructType* GetStruct() const\
	{\
		return Type();\
	}\
	virtual CE::String GetStructBundle() const\
	{\
		return StaticBundle();\
	}\
	virtual CE::String GetStructModule() const\
	{\
		return StaticModule();\
	}

