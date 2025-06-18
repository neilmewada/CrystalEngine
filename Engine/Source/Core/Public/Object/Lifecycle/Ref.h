#pragma once

namespace CE
{

    template<typename T = Object>
	class Ref final
	{
	public:
        
        Ref()
        {
            
        }
        
        Ref(T* object)
        {
            if (object != nullptr && object->control != nullptr)
            {
                control = object->control;
                control->AddStrongRef();

#if CE_BUILD_DEBUG
                ptr = object;
#endif
            }
        }
        
        template<class U> requires TIsBaseClassOf<T, U>::Value and (not std::is_same_v<T, U>)
        Ref(U* object)
        {
            if (object != nullptr && object->control != nullptr)
            {
                control = object->control;
                control->AddStrongRef();

#if CE_BUILD_DEBUG
                ptr = (T*)object;
#endif
            }
        }
        
        Ref(const Ref& copy)
        {
            control = copy.control;
            if (control)
            {
                control->AddStrongRef();
            }
#if CE_BUILD_DEBUG
            ptr = (T*)copy.ptr;
#endif
        }
        
        Ref& operator=(const Ref& copy)
        {
            if (control)
            {
                control->ReleaseStrongRef();
            }
            control = copy.control;
            if (control)
            {
                control->AddStrongRef();
            }
#if CE_BUILD_DEBUG
            ptr = (T*)copy.ptr;
#endif
            return *this;
        }
        
        Ref& operator=(T* newObject)
        {
            if (control)
            {
                control->ReleaseStrongRef();
            }
            control = nullptr;
            if (newObject != nullptr)
            {
                control = newObject->control;
            }
            if (control != nullptr)
            {
                control->AddStrongRef();
            }
#if CE_BUILD_DEBUG
            ptr = (T*)newObject;
#endif
            return *this;
        }
        
        template<class U> requires TIsBaseClassOf<T, U>::Value and (not std::is_same_v<T, U>)
        Ref& operator=(U* newObject)
        {
            if (control)
            {
                control->ReleaseStrongRef();
            }
            control = nullptr;
            if (newObject != nullptr)
            {
                control = newObject->control;
            }
            if (control != nullptr)
            {
                control->AddStrongRef();
            }
#if CE_BUILD_DEBUG
            ptr = (T*)newObject;
#endif
            return *this;
        }
        
        template<class U> requires TIsBaseClassOf<T, U>::Value
        Ref(const Ref<U>& copy)
        {
            control = copy.control;
            if (control)
            {
                control->AddStrongRef();
            }
#if CE_BUILD_DEBUG
            ptr = (T*)copy.ptr;
#endif
        }

        template<class U> requires TIsBaseClassOf<T, U>::Value
        Ref& operator=(const Ref<U>& copy)
        {
            if (control)
            {
                control->ReleaseStrongRef();
            }
            control = copy.control;
            if (control)
            {
                control->AddStrongRef();
            }
#if CE_BUILD_DEBUG
            ptr = (T*)copy.ptr;
#endif
            return *this;
        }
        
        Ref(Ref&& move)
        {
            control = move.control;
            move.control = nullptr;
#if CE_BUILD_DEBUG
            ptr = (T*)move.ptr;
            move.ptr = nullptr;
#endif
        }

        //! @brief This will cast type from T to U
        template<class U> requires TIsBaseClassOf<Object, U>::Value and (not std::is_same_v<T, U>)
        explicit operator Ref<U>() const
        {
            return Ref<U>((U*)Get());
        }
        
        ~Ref()
        {
            if (control)
            {
                control->ReleaseStrongRef();
                control = nullptr;
            }
#if CE_BUILD_DEBUG
            ptr = nullptr;
#endif
        }
        
        inline T* Get() const
        {
            Object* object = nullptr;
            if (control == nullptr)
            {
                return nullptr;
            }
            object = control->GetObject();
            if (object == nullptr)
            {
                return nullptr;
            }
            return (T*)object;
        }
        
        inline T& operator*() const
        {
            Object* object = nullptr;
            if (control == nullptr)
            {
                throw NullPointerException("Reference is NULL!");
            }
            object = control->GetObject();
            if (object == nullptr)
            {
                throw NullPointerException("Object is destroyed!");
            }
            return *(T*)object;
        }
        
        inline T* operator->() const
        {
            Object* object = nullptr;
            if (control == nullptr)
            {
                throw NullPointerException("Reference is NULL!");
            }
            object = control->GetObject();
            if (object == nullptr)
            {
                throw NullPointerException("Object is destroyed!");
            }
            return (T*)object;
        }

        bool IsNull() const
        {
            Object* object = nullptr;
            if (control == nullptr)
            {
                return true;
            }

            object = control->GetObject();

            return object == nullptr;
        }

        //! @brief Returns true if the pointer is neither a nullptr nor a destroyed object. 
        bool IsValid() const
        {
            return !IsNull();
        }
        
        inline bool operator!() const
        {
            return !IsValid();
        }
        
        inline operator bool() const
        {
            return IsValid();
        }

        inline bool operator==(T* object) const
        {
            return (control == nullptr && object == nullptr) ||
                (control != nullptr && control->GetObject() == object);
        }

        inline bool operator!=(T* object) const
        {
            return !operator==(object);
        }
        
        inline bool operator==(const Ref& rhs) const
        {
            return control == rhs.control;
        }
        
        inline bool operator!=(const Ref& rhs) const
        {
            return control != rhs.control;
        }

        bool operator==(const WeakRef<T>& rhs) const;

        inline bool operator!=(const WeakRef<T>& rhs) const
        {
            return !operator==(rhs);
        }

        template<typename U> requires TIsBaseClassOf<T, U>::Value or TIsBaseClassOf<U, T>::Value
        bool operator==(const Ref<U>& rhs) const
        {
            return control == rhs.control;
        }

        template<typename U> requires TIsBaseClassOf<T, U>::Value or TIsBaseClassOf<U, T>::Value
        bool operator!=(const Ref<U>& rhs) const
        {
            return !operator==(rhs);
        }

        template<typename U> requires TIsBaseClassOf<T, U>::Value or TIsBaseClassOf<U, T>::Value
        bool operator==(const WeakRef<U>& rhs) const;

        template<typename U> requires TIsBaseClassOf<T, U>::Value or TIsBaseClassOf<U, T>::Value
        bool operator!=(const WeakRef<U>& rhs) const
        {
            return !operator==(rhs);
        }

        SIZE_T GetHash() const
        {
            return (SIZE_T)control;
        }

	private:

#if CE_BUILD_DEBUG
        T* ptr = nullptr;
#endif
		Internal::RefCountControl* control = nullptr;

        template<typename U>
        friend class WeakRef;

        template<typename U>
        friend class Ref;
	};

    
} // namespace CE

