#pragma once

#include <type_traits>

#include "CrystalTemplate.h"

namespace CE
{
	class Object;

	enum class RefType
	{
		None = 0,
		Strong,
		Weak
	};
	ENUM_CLASS(RefType);

	template<typename T>
	struct TIsIntegralType : TFalseType {};

	template<>
	struct TIsIntegralType<u8> : TTrueType {};
	template<>
	struct TIsIntegralType<u16> : TTrueType {};
	template<>
	struct TIsIntegralType<u32> : TTrueType {};
	template<>
	struct TIsIntegralType<u64> : TTrueType {};
	template<>
	struct TIsIntegralType<s8> : TTrueType {};
	template<>
	struct TIsIntegralType<s16> : TTrueType {};
	template<>
	struct TIsIntegralType<s32> : TTrueType {};
	template<>
	struct TIsIntegralType<s64> : TTrueType {};

	template<typename T>
	struct TIsNumericType : TFalseType {};

	template<>
	struct TIsNumericType<u8> : TTrueType {};
	template<>
	struct TIsNumericType<u16> : TTrueType {};
	template<>
	struct TIsNumericType<u32> : TTrueType {};
	template<>
	struct TIsNumericType<u64> : TTrueType {};
	template<>
	struct TIsNumericType<s8> : TTrueType {};
	template<>
	struct TIsNumericType<s16> : TTrueType {};
	template<>
	struct TIsNumericType<s32> : TTrueType {};
	template<>
	struct TIsNumericType<s64> : TTrueType {};
	template<>
	struct TIsNumericType<f32> : TTrueType {};
	template<>
	struct TIsNumericType<f64> : TTrueType {};

	template <class, template <class> class>
	struct TypeIsInstanceOf : TFalseType {};

	template <class T, template <class> class U>
	struct TypeIsInstanceOf<U<T>, U> : TTrueType {};

	template<typename T>
	using RemovePointerFromType = std::remove_pointer_t<T>;

	template<typename T>
	using RemoveConstVolatileFromType = std::remove_cv_t<T>;

	template<typename T>
	using RemoveReferenceFromType = std::remove_reference_t<T>;

	template<class TBase, class TDerived>
	using TIsBaseClassOf = TBoolConst<__is_base_of(TBase, TDerived)>;

	template<typename T1, typename T2>
	struct TIsSameType : TFalseType {};

	template<typename T>
	struct TIsSameType<T, T> : TTrueType {};

    template<typename T>
    struct TIsTemplate : TFalseType {};

    template<typename T, template <typename> class U>
    struct TIsTemplate<U<T>> : TTrueType {};

	template<typename T>
	class Array;

	template<typename T>
	class SubClassType;

	template<typename T>
	class ScriptDelegate;

	template<typename T>
	class ScriptEvent;

	class String;
	class Name;

	template<typename T>
	struct PropertyBinding;

	template<typename T>
	struct TIsPropertyBinding : TFalseType
	{
		
	};

	template<typename T>
	struct TIsPropertyBinding<PropertyBinding<T>> : TTrueType
	{

	};

	template<typename T>
	struct TIsString : TFalseType
	{
		
	};

	template<>
	struct TIsString<char*> : TTrueType
	{

	};

	template<>
	struct TIsString<const char*> : TTrueType
	{

	};

	template<SIZE_T N>
	struct TIsString<const char[N]> : TTrueType
	{

	};

	template<SIZE_T N>
	struct TIsString<const char(&)[N]> : TTrueType
	{

	};

	template<SIZE_T N>
	struct TIsString<char[N]> : TTrueType
	{

	};

	template<SIZE_T N>
	struct TIsString<char(&)[N]> : TTrueType
	{

	};

	template<typename T>
	struct TIsString<const T&> : TIsString<T>
	{

	};

	template<typename T>
	struct TIsString<const T> : TIsString<T>
	{

	};

	template<typename T>
	struct TIsString<T&> : TIsString<T>
	{

	};

	template<>
	struct TIsString<String> : TTrueType
	{

	};

	template<typename T>
	struct TIsArray : TFalseType
	{
        using ElementType = void;
	};

	template<typename T>
	struct TIsArray<Array<T>> : TTrueType
	{
		using ElementType = T;
	};

	template<typename T>
	struct TIsSubClassType : TFalseType
	{
		using RequiredType = void;
	};

	template<typename T>
	struct TIsSubClassType<SubClassType<T>> : TTrueType
	{
		using RequiredType = T;
	};

	template<typename T>
	struct TIsScriptDelegate : TFalseType
	{
		using Signature = void;
	};

	template<typename T>
	struct TIsScriptDelegate<ScriptDelegate<T>> : TTrueType
	{
		using Signature = T;
	};

	template<typename T>
	struct TIsScriptEvent : TFalseType
	{
		using Signature = void;
	};

	template<typename T>
	struct TIsScriptEvent<ScriptEvent<T>> : TTrueType
	{
		using Signature = T;
	};

    template<typename T>
    struct TIsEnum : TBoolConst<__is_enum(T)> {};

    template<typename T, bool IsEnum = TIsEnum<T>::Value>
    struct TEnumUnderlyingType : std::underlying_type<T>
    {
	    typedef void Type;
    };

	template<typename T>
	struct TEnumUnderlyingType<T, true> : std::underlying_type<T>
	{
		typedef __underlying_type(T) Type;
	};

	template<typename T>
	class Ref;

	template<typename T>
	class WeakRef;

	template<class T>
	class IntrusivePtr;

	template<typename T>
	struct TIsRef : TFalseType
	{
		typedef void Type;
	};

	template<typename T>
	struct TIsRef<Ref<T>> : TTrueType
	{
		typedef T Type;
	};

	template<typename T>
	struct TIsWeakRef : TFalseType
	{
		typedef void Type;
	};

	template<typename T>
	struct TIsWeakRef<WeakRef<T>> : TTrueType
	{
		typedef T Type;
	};

	template<typename T>
	struct TPtrType : TFalseType
	{
		typedef void Type;

		static T* GetRawPtr(T ptr) { return nullptr; }
	};

	template<typename T>
	struct TPtrType<T*> : TTrueType
	{
		typedef T Type;

		static T* GetRawPtr(T* ptr) { return ptr; }
	};

	template<typename T>
	struct TPtrType<Ref<T>> : TTrueType
	{
		typedef T Type;

		static T* GetRawPtr(Ref<T> ptr);
	};

	template<typename T>
	struct TPtrType<WeakRef<T>> : TTrueType
	{
		typedef T Type;

		static T* GetRawPtr(WeakRef<T> ptr);
	};

	template<typename T>
	struct TPtrType<IntrusivePtr<T>> : TTrueType
	{
		typedef T Type;

		static T* GetRawPtr(IntrusivePtr<T> ptr);
	};

	template<typename T>
	struct TRefCounted : TFalseType
	{
		typedef void ReferencedType;

		static RefType GetRefType() { return RefType::None; }
	};

	template<typename T>
	struct TRefCounted<Ref<T>> : TTrueType
	{
		typedef T ReferencedType;
		
		static RefType GetRefType() { return RefType::Strong; }
	};

	template<typename T>
	struct TRefCounted<WeakRef<T>> : TTrueType
	{
		typedef T ReferencedType;

		static RefType GetRefType() { return RefType::Weak; }
	};

	template<typename T>
	struct TRefCounted<Array<Ref<T>>> : TTrueType
	{
		typedef T ReferencedType;

		static RefType GetRefType() { return RefType::Strong; }
	};

	template<typename T>
	struct TRefCounted<Array<WeakRef<T>>> : TTrueType
	{
		typedef T ReferencedType;

		static RefType GetRefType() { return RefType::Weak; }
	};

	template<typename T, bool IsEnum = TIsEnum<T>::Value, bool IsArray = TIsArray<T>::Value>
	struct TGetUnderlyingType : TFalseType
	{
		typedef void Type;
	};

	template<typename T>
	struct TGetUnderlyingType<T, true, false> : TTrueType // ENUM
	{
		typedef __underlying_type(T) Type;
	};

	template<typename T>
	struct TGetUnderlyingType<Array<T>, false, true> : TTrueType // Array<T>
	{
		typedef T Type;
	};

	template<typename T>
	struct TGetUnderlyingType<Ref<T>, false, false> : TTrueType // Ref<T>
	{
		typedef void Type;
	};

	template<typename T>
	struct TGetUnderlyingType<WeakRef<T>, false, false> : TTrueType // WeakRef<T>
	{
		typedef void Type;
	};

	template<typename T>
	struct TGetUnderlyingType<SubClassType<T>, false, false> : TTrueType // SubClassType<T>
	{
		typedef T Type;
	};

	template<typename T, template <typename> class U>
	struct TGetUnderlyingType<U<T>, false, false> : TTrueType // SubClassType<T>
	{
		typedef T Type;
	};

	class ObjectMap;
    
	template<typename T>
	struct IsObjectStoreType : TFalseType {};

	template<>
	struct IsObjectStoreType<ObjectMap> : TTrueType {};

	template<typename T>
	struct TIsAbstract : TBoolConst<__is_abstract(T)> {};

	template<typename T>
	struct TIsClass : TBoolConst<__is_class(T) && T::IsClass> {};

	template<typename T, typename = void>
	struct TIsStruct : TBoolConst<false> {};

	template<typename T>
	struct TIsStruct<T, std::void_t<decltype(T::IsStruct)>> : TBoolConst<__is_class(T) && T::IsStruct> {};

	template<typename What, typename... Args>
	struct TIsTypePresent : TBoolConst<(std::is_same_v<What, Args> or ...)>
	{
		
	};

	// Function

	template <typename T, typename = void>
	struct TFunctionTraits
	{
		static constexpr bool Value = false;
	};

	template <typename T>
	struct TFunctionTraits<T, std::void_t<decltype(&T::operator())>> : TFunctionTraits<decltype(&T::operator())>
	{};

	// For generic types, directly use the result of the signature of its 'operator()'

	template <typename TClassType, typename TReturnType, typename... Args>
	struct TFunctionTraits<TReturnType(TClassType::*)(Args...) const> // we specialize for pointers to const member function
	{
		enum { NumArgs = sizeof...(Args) };

		static constexpr bool Value = true;

		typedef TReturnType ReturnType;
		typedef TClassType ClassType;
		
		typedef TReturnType(TClassType::* FuncSignature)(Args...) const;

		typedef std::tuple<Args...> Tuple;

		template <SIZE_T i>
		struct Arg
		{
			typedef typename std::tuple_element<i, std::tuple<Args...>>::type Type;
			// the i-th argument is equivalent to the i-th tuple element of a tuple
			// composed of those arguments.
		};
	};

	template <typename TClassType, typename TReturnType, typename... Args>
	struct TFunctionTraits<TReturnType(TClassType::*)(Args...)> // we specialize for pointers to member function
	{
		enum { NumArgs = sizeof...(Args) };

		static constexpr bool Value = true;

		typedef TReturnType ReturnType;
		typedef TClassType ClassType;

		typedef TReturnType(TClassType::* FuncSignature)(Args...);

		typedef std::tuple<Args...> Tuple;

		template <SIZE_T i>
		struct Arg
		{
			typedef typename std::tuple_element<i, std::tuple<Args...>>::type Type;
			// the i-th argument is equivalent to the i-th tuple element of a tuple
			// composed of those arguments.
		};
	};

	template <typename TReturnType, typename... Args>
	struct TFunctionTraits<TReturnType(*)(Args...)> // we specialize for pointers to global functions
	{
		enum { NumArgs = sizeof...(Args) };

		static constexpr bool Value = true;

		typedef TReturnType ReturnType;
		typedef void ClassType;

		typedef TReturnType(* FuncSignature)(Args...);

		typedef std::tuple<Args...> Tuple;

		template <SIZE_T i>
		struct Arg
		{
			typedef typename std::tuple_element<i, std::tuple<Args...>>::type Type;
			// the i-th argument is equivalent to the i-th tuple element of a tuple
			// composed of those arguments.
		};
	};

	template<class TCastClassType, typename T, typename = void>
	struct TMemberFunctionCast : TFalseType
	{

	};

	template<class TCastClassType, typename TRetType, class TClassType, class... TArgs>
	struct TMemberFunctionCast<TCastClassType, TRetType(TClassType::*)(TArgs...)>
	{
		using TCastedFuncTraits = TFunctionTraits<TRetType(TCastClassType::*)(TArgs...)>;
		using TCastedFuncSignature = TCastedFuncTraits::FuncSignature;

		static constexpr bool Value = TCastedFuncTraits::Value;
	};

	template<typename T, typename = void>
	struct TStructReleaseFunction : TFalseType
	{
		static void Release(T* instance) {} // Do nothing
	};

	template<typename T>
	struct TStructReleaseFunction<T, std::void_t<decltype(std::declval<T>().Release())>> : TTrueType
	{
		static void Release(T* instance)
		{
			instance->T::Release(); // Release function exists
		}
	};

	/// True if type has default constructor and is NOT abstract
	template<typename T, typename = void>
	struct THasDefaultCtor : TFalseType
	{

	};

	template<typename T>
	struct THasDefaultCtor<T, std::void_t<decltype(T())>> : TTrueType
	{

	};

	template<typename T>
	struct TIsCopyConstructible : TBoolConst<std::is_copy_constructible<T>::value>
	{

	};

	template<typename T>
	struct TIsCopyable : TBoolConst<std::is_copy_constructible_v<T> and std::is_copy_assignable_v<T>>
	{

	};

	template<typename T, typename = void>
	struct THasGetHashFunction : TFalseType
	{
		static SIZE_T GetHash(const T* instance) { return 0; }
	};

	template<typename T>
	struct THasGetHashFunction<T, std::void_t<decltype(std::declval<T>().GetHash())>> : TTrueType
	{
		static SIZE_T GetHash(const T* instance) { return instance->GetHash(); }
	};

	template<typename T, typename = void>
	struct THasOnBeforeSerializeFunction : TFalseType
	{
		static void OnBeforeSerialize(T* instance) {} // Do nothing
	};

	template<typename T>
	struct THasOnBeforeSerializeFunction<T, std::void_t<decltype(std::declval<T>().OnBeforeSerialize())>> : TTrueType
	{
		static void OnBeforeSerialize(T* instance) { return instance->OnBeforeSerialize(); }
	};

	template<typename T, typename = void>
	struct THasOnAfterDeserializeFunction : TFalseType
	{
		static void OnAfterDeserialize(T* instance) {} // Do nothing
	};

	template<typename T>
	struct THasOnAfterDeserializeFunction<T, std::void_t<decltype(std::declval<T>().OnAfterDeserialize())>> : TTrueType
	{
		static void OnAfterDeserialize(T* instance) { return instance->OnAfterDeserialize(); }
	};

	template<typename T>
	struct TTypeDestructor
	{
		static void Invoke(void* instance) 
		{
			static_assert(std::is_destructible<T>::value);
			((T*)instance)->~T();
		}
	};

	template<typename T, typename... List>
	struct TContainsType : TFalseType
	{};

	template<typename T, typename... Rest>
	struct TContainsType<T, T, Rest...> : TTrueType
	{};

	template<typename T, typename First, typename... Rest>
	struct TContainsType<T, First, Rest...> : TContainsType<T, Rest...>
	{};

	template<typename T, typename... List>
	struct TMatchAllBaseClass : TFalseType
	{};

	template<typename T>
	struct TMatchAllBaseClass<T> : TTrueType
	{};

	template<typename T, typename First, typename... Rest>
	struct TMatchAllBaseClass<T, First, Rest...> : TBoolConst<TIsBaseClassOf<T, First>::Value and TMatchAllBaseClass<T, Rest...>::Value>
	{};

	template <typename T>
	struct TEquitable : TBoolConst<std::equality_comparable<T>>
	{
		static bool AreEqual(const T& lhs, const T& rhs)
		{
			return lhs == rhs;
		}
	};

} // namespace CE::Traits

