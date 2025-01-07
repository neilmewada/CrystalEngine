#pragma once

#define FUNCTION_BINDING(objectPtr, functionName, ...) \
	CE::FunctionBinding(objectPtr,\
		std::remove_cvref_t<decltype(*objectPtr)>::Type()\
			->FindFunction<TFunctionTraits<decltype(&std::remove_cvref_t<decltype(*objectPtr)>::functionName)>::ReturnType, \
				TFunctionTraits<decltype(&std::remove_cvref_t<decltype(*objectPtr)>::functionName)>::ClassType,\
                ##__VA_ARGS__>(#functionName,\
					&std::remove_cvref_t<decltype(*objectPtr)>::functionName))

#define DECLARE_SCRIPT_EVENT(EventName, ...) \
    typedef CE::ScriptEvent<void(__VA_ARGS__)> EventName;
#define DECLARE_SCRIPT_DELEGATE(EventName, ReturnType, ...) \
    typedef CE::ScriptDelegate<ReturnType(__VA_ARGS__)> EventName;

namespace CE
{
    class Object;
    class FunctionType;

    struct FunctionBinding
    {
        FunctionBinding()
        {}

        FunctionBinding(const WeakRef<Object>& object, FunctionType* function)
	        : object(object), function(function)
        {}

        WeakRef<Object> object = nullptr;
        FunctionType* function = nullptr;
    };

    template<typename T = void(void)>
    class ScriptDelegate
    {
    public:

        static_assert(CE::Internal::AlwaysFalse<T>, "Delegate should always be passed with function template arguments.");

    };

    class IScriptDelegate
    {
    public:

        virtual Variant Invoke(const Array<Variant>& args) const = 0;

        virtual SIZE_T GetSignature() const = 0;
        virtual Array<TypeId> GetParamterTypes() = 0;

        virtual void CopyFrom(IScriptDelegate* other) = 0;

        virtual void Bind(const Ref<Object>& object, FunctionType* function) = 0;
        virtual void Bind(const FunctionBinding& binding) = 0;
        virtual void Bind(const Delegate<Variant(const Array<Variant>&)>& lambda) = 0;

        virtual void Unbind() = 0;

        virtual bool IsBound() const = 0;
        virtual bool IsFunction() const = 0;
        virtual bool IsLambda() const = 0;

        virtual FunctionBinding GetBinding() const = 0;

        virtual DelegateHandle GetLambdaHandle() const = 0;

        virtual void Invalidate() = 0;

    protected:

        IScriptDelegate() = default;
        virtual ~IScriptDelegate() = default;

        friend class Object;
    };

    template<class TRetType, class... TArgs>
    class ScriptDelegate<TRetType(TArgs...)> : public IScriptDelegate
    {
    public:

        ScriptDelegate()
        {}

        ScriptDelegate(std::nullptr_t)
        {}

        ScriptDelegate(const Ref<Object>& object, FunctionType* function)
        {
            this->dstObject = object;
            this->dstFunction = function;

            isBound = dstObject && dstFunction;
        }

        ScriptDelegate(const FunctionBinding& binding) : ScriptDelegate(binding.object.Lock(), binding.function)
        {

        }

    private:

        template<typename F, std::size_t... Is>
        ScriptDelegate(const F& lambda, std::index_sequence<Is...>)
        {
            using Traits = TFunctionTraits<F>;
            using TraitsRetType = typename Traits::ReturnType;
            
            this->lambda = [lambda](const Array<Variant>& params) -> Variant
                {
                    if constexpr (TIsSameType<TraitsRetType, void>::Value)
                    {
                        lambda((params.begin() + Is)->GetValue<typename Traits::template Arg<Is>::Type>()...);
                        return Variant();
                    }
                    else
                    {
                        return lambda((params.begin() + Is)->GetValue<typename Traits::template Arg<Is>::Type>()...);
                    }
                };

            isBound = this->lambda.IsValid();
        }

    public:

        template<typename F>
        ScriptDelegate(const F& lambda) : ScriptDelegate(lambda, std::make_index_sequence<TFunctionTraits<F>::NumArgs>())
        {
            
        }

        ScriptDelegate(const Delegate<Variant(const Array<Variant>&)>& lambda)
        {
            this->lambda = lambda;
            isBound = this->lambda.IsValid();
        }

        ScriptDelegate(const ScriptDelegate& copy)
        {
            CopyFrom(copy);
        }

        ScriptDelegate(ScriptDelegate&& move) noexcept
        {
            MoveFrom(move);
        }

        ScriptDelegate& operator=(const ScriptDelegate& copy)
        {
            CopyFrom(copy);
            return *this;
        }

        ScriptDelegate& operator=(ScriptDelegate&& move) noexcept
        {
            MoveFrom(move);
            return *this;
        }

        Variant Invoke(const Array<Variant>& args) const override;

        TRetType Invoke(const TArgs&... args) const;

        TRetType operator()(const TArgs&... args) const
        {
            return Invoke(args...);
        }

        SIZE_T GetSignature() const override
        {
            return CE::GetFunctionSignature<TArgs...>();
        }

        Array<TypeId> GetParamterTypes() override
        {
            return { CE::GetTypeId<TArgs>()... };
        }

        void CopyFrom(IScriptDelegate* other) override
        {
	        if (other == nullptr || other->GetSignature() != GetSignature())
                return;

            ScriptDelegate* otherDelegate = (ScriptDelegate*)other;

            *this = *otherDelegate;
        }

        bool IsBound() const override
        {
            return isBound;
        }

        bool IsFunction() const override
        {
            return isBound && dstObject != nullptr && dstFunction != nullptr;
        }

        bool IsLambda() const override
        {
            return isBound && lambda.IsValid() && !IsFunction();
        }

        void Invalidate() override
        {
            isBound = false;
        }

        DelegateHandle GetLambdaHandle() const override
        {
	        if (lambda.IsValid())
	        {
                return lambda.GetHandle();
	        }
            return 0;
        }

        FunctionBinding GetBinding() const override
        {
            if (!isBound)
                return FunctionBinding();
            return FunctionBinding(dstObject, dstFunction);
        }

        void Bind(const Ref<Object>& object, FunctionType* function) override
        {
            if (object == nullptr || function == nullptr)
                return;
            if (this->GetSignature() != function->GetFunctionSignature())
                return;

            dstObject = object;
            dstFunction = function;

            isBound = true;
        }

		void Bind(const FunctionBinding& binding) override
        {
            Bind(binding.object.Lock(), binding.function);
        }

		void Bind(const Delegate<Variant(const Array<Variant>&)>& lambda) override
        {
            this->lambda = lambda;
            isBound = this->lambda.IsValid();
        }

		void Unbind() override
        {
            this->lambda = nullptr;
            dstObject = nullptr;
            dstFunction = nullptr;
            isBound = false;
        }

        template<typename TLambda>
        DelegateHandle Bind(const TLambda& lambda)
        {
            return BindInternal(lambda, std::make_index_sequence<sizeof...(TArgs)>());
        }

    private:

        template<typename TLambda, std::size_t... Is>
        DelegateHandle BindInternal(const TLambda& lambda, std::index_sequence<Is...>)
        {
            Delegate<Variant(const Array<Variant>&)> delegate = [lambda](const Array<Variant>& variantArgs) -> Variant
                {
                    if constexpr (TFunctionTraits<TLambda>::NumArgs == 0)
                    {
                        lambda();
                    }
                    else
                    {
                        lambda((variantArgs.begin() + Is)->GetValue<TArgs>()...);
                    }
                    return nullptr;
                };
            Bind(delegate);
            return delegate.GetHandle();
        }

        void CopyFrom(const ScriptDelegate& copy)
        {
            dstFunction = copy.dstFunction;
            dstObject = copy.dstObject;
            lambda = copy.lambda;
            isBound = copy.isBound;
        }

        void MoveFrom(ScriptDelegate& move)
        {
            dstFunction = move.dstFunction;
            dstObject = move.dstObject;
            lambda = move.lambda;
            isBound = move.isBound;

            move.dstFunction = nullptr;
            move.dstObject = nullptr;
            move.lambda = {};
            move.isBound = false;
        }

        FunctionType* dstFunction = nullptr;
        WeakRef<Object> dstObject = nullptr;
        Delegate<Variant(const Array<Variant>&)> lambda{};
        mutable bool isBound = false;

        template<typename T>
        friend class ScriptEvent;

        friend class Object;
    };

    template<typename T = void(void)>
    class ScriptEvent
    {
    public:
        static_assert(CE::Internal::AlwaysFalse<T>, "Delegate should always be passed with function template arguments.");

    };

    class IScriptEvent
    {
    public:

        virtual void Broadcast(const Array<Variant>& args) const = 0;

        virtual SIZE_T GetSignature() const = 0;

        virtual Array<TypeId> GetParameterTypes() = 0;

        virtual void CopyFrom(IScriptEvent* other) = 0;

        virtual u32 GetInvocationListCount() = 0;

        virtual void Bind(const Ref<Object>& object, FunctionType* function) = 0;
        virtual void Bind(const FunctionBinding& binding) = 0;
        virtual void Bind(const Delegate<Variant(const Array<Variant>&)>& lambda) = 0;

        virtual void Unbind(const Ref<Object>& object, FunctionType* function) = 0;
        virtual void Unbind(const FunctionBinding& binding) = 0;
        virtual void Unbind(DelegateHandle lambdaHandle) = 0;

        virtual void Unbind(const Delegate<Variant(const Array<Variant>&)>& lambda)
        {
	        if (lambda.IsValid())
	        {
                Unbind(lambda.GetHandle());
	        }
        }

        virtual void UnbindAll(Object* object) = 0;
        virtual void UnbindAll() = 0;

        virtual Array<FunctionBinding> GetSerializableBindings() = 0;

    protected:

        IScriptEvent() = default;
        virtual ~IScriptEvent() = default;

        IScriptEvent(const IScriptEvent&) {}
        IScriptEvent& operator=(const IScriptEvent&) { return *this; }

        IScriptEvent(IScriptEvent&&) noexcept {}
        IScriptEvent& operator=(IScriptEvent&&) noexcept { return *this; }

        friend class Object;
    };

    template<class... TArgs>
    class ScriptEvent<void(TArgs...)> final : public IScriptEvent
    {
    public:

        using DelegateType = ScriptDelegate<void(TArgs...)>;
        using InvocationListType = List<DelegateType>;

        constexpr static int NumArgs = sizeof...(TArgs);

        void Broadcast(const Array<Variant>& args) const override
        {
            isBroadcasting = true;

            for (int i = 0; i < invocationList.GetSize(); ++i)
            {
                if (invocationList[i].IsBound())
                {
	                invocationList[i].Invoke(args);
                }
            }

            isBroadcasting = false;
        }

        void Broadcast(const TArgs&... args) const
        {
            if (invocationList.IsEmpty())
                return;

            Broadcast(Array<Variant>{ args... });
        }

        void operator()(const TArgs&... args) const
        {
            if (invocationList.IsEmpty())
                return;

            Broadcast(Array<Variant>{ args... });
        }

        SIZE_T GetSignature() const override
        {
	        if (signature == 0)
	        {
                signature = CE::GetFunctionSignature<TArgs...>();
	        }
            return signature;
        }

        Array<TypeId> GetParameterTypes() override
        {
            return { CE::GetTypeId<TArgs>()... };
        }

        void CopyFrom(IScriptEvent* other) override;

        void Bind(const Ref<Object>& object, FunctionType* function) override
        {
	        if (object == nullptr || function == nullptr)
                return;
            if (function->GetFunctionSignature() != GetSignature())
                return;

            invocationList.EmplaceBack(object, function);
        }

        void Bind(const FunctionBinding& binding) override
        {
            Bind(binding.object.Lock(), binding.function);
        }

        void Bind(const Delegate<Variant(const Array<Variant>&)>& lambda) override
        {
	        if (!lambda.IsValid())
                return;

            invocationList.EmplaceBack(lambda);
        }

        template<typename TLambda> requires (!TIsSameType<TLambda, DelegateType>::Value)
        DelegateHandle Bind(const TLambda& lambda)
        {
            return BindInternal(lambda, std::make_index_sequence<sizeof...(TArgs)>());
        }

        void Bind(const DelegateType& delegate)
        {
	        if (delegate.IsBound())
	        {
                invocationList.Add(delegate);
	        }
        }

        void Unbind(const Ref<Object>& object, FunctionType* function) override
        {
            if (object == nullptr || function == nullptr)
                return;

            for (int i = 0; i < invocationList.GetSize(); ++i)
            {
	            if (invocationList[i].dstObject == object && invocationList[i].dstFunction == function)
	            {
                    invocationList[i].Invalidate();
		            break;
	            }
            }
        }

        void Unbind(const FunctionBinding& binding) override
        {
            for (int i = 0; i < invocationList.GetSize(); ++i)
            {
                if (invocationList[i].dstObject == binding.object && invocationList[i].dstFunction == binding.function)
                {
                    invocationList[i].Invalidate();
                    break;
                }
            }
        }

        void Unbind(DelegateHandle lambdaHandle) override
        {
            for (int i = 0; i < invocationList.GetSize(); ++i)
            {
	            if (invocationList[i].lambda.IsValid() && invocationList[i].lambda.GetHandle() == lambdaHandle)
	            {
                    invocationList[i].Invalidate();
		            break;
	            }
            }
        }

        void UnbindAll(Object* object) override
        {
            for (int i = 0; i < invocationList.GetSize(); ++i)
            {
                if (invocationList[i].dstObject == object)
                {
                    invocationList[i].Invalidate();
                }
            }
        }

        void UnbindAll() override
        {
	        for (int i = 0; i < invocationList.GetSize(); ++i)
	        {
                invocationList[i].Invalidate();
	        }

            if (!isBroadcasting)
            {
                invocationList.Clear();
            }
        }

        Array<FunctionBinding> GetSerializableBindings() override
        {
            Array<FunctionBinding> result;

            for (const DelegateType& item : invocationList)
            {
	            if (item.IsBound() && item.IsFunction())
	            {
                    result.Add(item.GetBinding());
	            }
            }

            return result;
        }

        u32 GetInvocationListCount() override
        {
            return invocationList.GetSize();
        }

        ScriptEvent() = default;
        virtual ~ScriptEvent() = default;

        // Operators

        void operator+=(const FunctionBinding& binding)
        {
            Bind(binding);
        }

        void operator+=(const Delegate<Variant(const Array<Variant>&)>& lamda)
        {
            Bind(lamda);
        }

        void operator-=(const FunctionBinding& binding)
        {
            Unbind(binding);
        }

        void operator-=(const Delegate<Variant(const Array<Variant>&)>& lamda)
        {
            if (lamda.IsValid())
            {
	            Unbind(lamda.GetHandle());
            }
        }

        void operator-=(DelegateHandle delegateHandle)
        {
            Unbind(delegateHandle);
        }

        template<typename TLambda>
        DelegateHandle operator+(const TLambda& lambda)
        {
            return BindInternal(lambda, std::make_index_sequence<sizeof...(TArgs)>());
        }

        template<typename TLambda>
        void operator+=(const TLambda& lambda)
        {
            BindInternal(lambda, std::make_index_sequence<sizeof...(TArgs)>());
        }

    private:

        template<typename TLambda, std::size_t... Is>
        DelegateHandle BindInternal(const TLambda& lambda, std::index_sequence<Is...>)
        {
            Delegate<Variant(const Array<Variant>&)> delegate = [lambda](const Array<Variant>& variantArgs) -> Variant
                {
					if constexpr (TFunctionTraits<TLambda>::NumArgs == 0)
					{
                        lambda();
					}
                    else
                    {
						lambda((variantArgs.begin() + Is)->GetValue<TArgs>()...);
                    }
                    return nullptr;
                };
            Bind(delegate);
            return delegate.GetHandle();
        }

        InvocationListType invocationList{};
        mutable bool isBroadcasting = false;
        mutable SIZE_T signature = 0;

        friend class Object;
    };

    template <class ... TArgs>
    void ScriptEvent<void(TArgs...)>::CopyFrom(IScriptEvent* other)
    {
        if (other->GetSignature() != GetSignature())
            return;

        ScriptEvent* otherEvent = (ScriptEvent*)other;

        for (const DelegateType& delegate : otherEvent->invocationList)
        {
            Bind(delegate);
        }
    }
    
} // namespace CE

CE_RTTI_POD_TEMPLATE(CORE_API, CE, ScriptDelegate, void(void))
CE_RTTI_POD_TEMPLATE(CORE_API, CE, ScriptEvent, void(void))
