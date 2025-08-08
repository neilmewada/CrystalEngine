#pragma once

#define FNewOwned(WidgetClass, Parent)\
	(*CreateObject<WidgetClass>(Parent, #WidgetClass))
#define FNewOwnedDynamic(WidgetClass, Parent, WidgetDynamicClass)\
	(*CreateObject<WidgetClass>(Parent, #WidgetClass, OF_NoFlags, WidgetDynamicClass))

#define FNew(WidgetClass)\
	(*CreateObject<WidgetClass>(this, #WidgetClass, OF_NoFlags))

#define FNewNamed(WidgetClass, name)\
	(*CreateObject<WidgetClass>(this, #WidgetClass, OF_NoFlags)).Name(name).As<WidgetClass>()

#define FNewDynamic(WidgetClass, WidgetDynamicClass)\
	(*CreateObject<WidgetClass>(this, #WidgetClass, OF_NoFlags, WidgetDynamicClass))

#define FAssignNew(WidgetClass, VariableName) FNew(WidgetClass).Assign(VariableName)
#define FAssignNewDynamic(WidgetClass, VariableName, WidgetDynamicClass) FNewDynamic(WidgetClass, WidgetDynamicClass).Assign(VariableName)
#define FAssignNewNamed(WidgetClass, VariableName, name) FNew(WidgetClass).Name(name).Assign(VariableName)

#define FAssignNewOwned(WidgetClass, VariableName, Parent) FNewOwned(WidgetClass, Parent).Assign(VariableName)
#define FAssignNewOwnedDynamic(WidgetClass, VariableName, Parent, WidgetDynamicClass) FNewOwnedDynamic(WidgetClass, Parent, WidgetDynamicClass).Assign(VariableName)


#define __FUSION_PROPERTY(PropertyType, PropertyName, DirtyFunc)\
	protected:\
		PropertyType m_##PropertyName = {};\
	public:\
		Self& PropertyName(PropertyType const& value) {\
			if constexpr (TEquitable<PropertyType>::Value)\
			{\
				if (TEquitable<PropertyType>::AreEqual(this->m_##PropertyName, value))\
					return *this;\
			}\
			this->m_##PropertyName = value;\
			thread_local const CE::Name nameValue = #PropertyName;\
			if ((GetFlags() & OF_InsideConstructor) == 0) {\
			OnFusionPropertyModified(nameValue);\
			DirtyFunc;}\
			return *this;\
		}\
		auto PropertyName() const { return this->m_##PropertyName; }

#define FUSION_LAYOUT_PROPERTY(PropertyType, PropertyName, ...) __FUSION_PROPERTY(PropertyType, PropertyName, MarkLayoutDirty())

#define FUSION_PROPERTY(PropertyType, PropertyName, ...) __FUSION_PROPERTY(PropertyType, PropertyName, MarkDirty())

#define __FUSION_DATA_PROPERTY(PropertyType, PropertyName, DirtyFunc)\
	__FUSION_PROPERTY(PropertyType, PropertyName, DirtyFunc)\
	PropertyBinding<PropertyType> m_##PropertyName##Binding{};\
	void Update_##PropertyName(CE::Object* modifyingObject = nullptr) {\
		if (m_##PropertyName##Binding.read.IsBound() && this != modifyingObject) PropertyName(m_##PropertyName##Binding.read());\
	}\
	Self& Bind_##PropertyName(const PropertyBindingRequest<PropertyType>& request) {\
		m_##PropertyName##Binding.read = request.read;\
		m_##PropertyName##Binding.write = request.write;\
		request.onModifiedExternally.Bind(FUNCTION_BINDING(this, Update_##PropertyName));\
		Update_##PropertyName(nullptr);\
		return *this;\
	}

#define FUSION_DATA_PROPERTY(PropertyType, PropertyName, ...) __FUSION_DATA_PROPERTY(PropertyType, PropertyName, MarkDirty())

#define FUSION_DATA_LAYOUT_PROPERTY(PropertyType, PropertyName, ...) __FUSION_DATA_PROPERTY(PropertyType, PropertyName, MarkLayoutDirty())

#define BIND_PROPERTY(modelPtr, propertyName, getter, setter) {\
		getter, setter,\
		modelPtr->On##propertyName##Updated()\
	}

#define BIND_PROPERTY_RW(modelPtr, propertyName) CE::PropertyBindingRequest<std::remove_cvref_t<TFunctionTraits<decltype(&std::remove_cvref_t<decltype(*modelPtr)>::Get##propertyName)>::ReturnType>>(\
	FUNCTION_BINDING(model, Get##propertyName),\
	FUNCTION_BINDING(model, Set##propertyName),\
	modelPtr->On##propertyName##Updated())

#define BIND_PROPERTY_R(modelPtr, propertyName) CE::PropertyBindingRequest<std::remove_cvref_t<TFunctionTraits<decltype(&std::remove_cvref_t<decltype(*modelPtr)>::Get##propertyName)>::ReturnType>>(\
	FUNCTION_BINDING(modelPtr, Get##propertyName),\
	nullptr,\
	modelPtr->On##propertyName##Updated())

#define MODEL_PROPERTY(PropertyType, PropertyName, ...) \
	protected:\
		PropertyType m_##PropertyName = {};\
		FUSION_EVENT(ScriptEvent<void(Object*)>, On##PropertyName##Updated)\
	public:\
		auto Get##PropertyName() const { return m_##PropertyName; }\
		void Set##PropertyName(const PropertyType& value, Object* modifyingObject = nullptr) {\
			m_##PropertyName = value; m_On##PropertyName##Updated(modifyingObject);\
			thread_local const CE::Name propName = #PropertyName;\
			THasOnModelPropertyUpdated<Self>::OnModelPropertyUpdated(this, propName, modifyingObject);\
		}

#define __WRAPPED_PROP_TYPE(PropertyName, WrappingVariable) std::remove_cvref_t<decltype(std::declval<std::remove_cvref_t<decltype(*WrappingVariable)>>().PropertyName())>

#define FUSION_PROPERTY_WRAPPER(PropertyName, WrappingVariable)\
	Self& PropertyName(const __WRAPPED_PROP_TYPE(PropertyName, WrappingVariable)& value) {\
		if (WrappingVariable == nullptr) return *this;\
		WrappingVariable->PropertyName(value);\
		return *this;\
	}\
	auto PropertyName() const { return WrappingVariable->PropertyName(); }

#define FUSION_PROPERTY_WRAPPER2(PropertyName, WrappingVariable, WrappedPropertyName)\
	Self& WrappedPropertyName(const __WRAPPED_PROP_TYPE(PropertyName, WrappingVariable)& value) {\
		if (WrappingVariable == nullptr) return *this;\
		WrappingVariable->PropertyName(value);\
		return *this;\
	}\
	auto WrappedPropertyName() const { return WrappingVariable->PropertyName(); }

#define FUSION_DATA_PROPERTY_WRAPPER(PropertyName, WrappingVariable)\
	FUSION_PROPERTY_WRAPPER(PropertyName, WrappingVariable)\
	Self& Bind_##PropertyName(const CE::PropertyBindingRequest<__WRAPPED_PROP_TYPE(PropertyName, WrappingVariable)>& request) {\
		WrappingVariable->Bind_##PropertyName(request);\
		return *this;\
	}

#define FUSION_EVENT(EventType, PropertyName, ...)\
	protected:\
		EventType m_##PropertyName;\
	public:\
		auto& PropertyName() { return m_##PropertyName; }\
		Self& PropertyName(const FunctionBinding& binding)\
		{\
			m_##PropertyName.Bind(binding);\
			return *this;\
		}\
		template<typename TLambda>\
		Self& PropertyName(const TLambda& lambda)\
		{\
			m_##PropertyName.Bind(lambda);\
			return *this;\
		}\
		template<typename TLambda>\
		Self& PropertyName(DelegateHandle& outHandle, const TLambda& lambda)\
		{\
			outHandle = m_##PropertyName.Bind(lambda);\
			return *this;\
		}

#define FUSION_EVENT_WRAPPER(PropertyName, WrappingVariable)\
	auto PropertyName() const { return WrappingVariable->PropertyName(); }\
	Self& PropertyName(const FunctionBinding& binding)\
	{\
		if (WrappingVariable == nullptr) return *this;\
		WrappingVariable->PropertyName(binding);\
		return *this;\
	}\
	template<typename TLambda>\
	Self& PropertyName(const TLambda& lambda)\
	{\
		if (WrappingVariable == nullptr) return *this;\
		WrappingVariable->PropertyName(lambda);\
		return *this;\
	}\
	template<typename TLambda>\
	Self& PropertyName(DelegateHandle& outHandle, const TLambda& lambda)\
	{\
		if (WrappingVariable == nullptr) return *this;\
		WrappingVariable->PropertyName(outHandle, lambda);\
		return *this;\
	}


#define FUSION_WIDGET\
	public:\
		template<typename TLambda> requires TValidate_IfTrue<TLambda>::Value\
		Self& IfTrue(bool condition, const TLambda& lambda)\
		{\
			if (condition) lambda(*this);\
			return *this;\
		}\
		FUSION_FRIENDS

namespace CE
{
	template<typename T, typename = void>
	struct THasOnModelPropertyUpdated : TFalseType
	{
		static void OnModelPropertyUpdated(T* instance, const CE::Name& propertyName, Object* modifyingObject) {} // Do nothing
	};

	template<typename T>
	struct THasOnModelPropertyUpdated<T, std::void_t<decltype(std::declval<T>().OnModelPropertyUpdated(CE::Name()))>> : TTrueType
	{
		static void OnModelPropertyUpdated(T* instance, const CE::Name& propertyName, Object* modifyingObject) { return instance->OnModelPropertyUpdated(propertyName, modifyingObject); }
	};

	template<typename T>
	struct PropertyBindingRequest
	{
		PropertyBindingRequest(const ScriptDelegate<T()>& read, const ScriptDelegate<void(const T&, Object*)>& write, ScriptEvent<void(Object*)>& onModifiedExternally)
			: read(read), write(write), onModifiedExternally(onModifiedExternally)
		{
			
		}

		const ScriptDelegate<T()>& read;
		const ScriptDelegate<void(const T&, Object*)>& write;
		ScriptEvent<void(Object*)>& onModifiedExternally;
	};

}
