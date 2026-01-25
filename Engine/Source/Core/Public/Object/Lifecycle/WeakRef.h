#pragma once

namespace CE
{
    
    template<typename T = Object>
    class WeakRef final
    {
    public:
        WeakRef()
        {
            
        }
        
        WeakRef(T* object)
        {
            if (object != nullptr && object->control != nullptr)
            {
                control = object->control;
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = object;
#endif
        }
        
        template<class U> requires TIsBaseClassOf<T, U>::Value and (not std::is_same_v<T, U>)
        WeakRef(U* object)
        {
            if (object != nullptr && object->control != nullptr)
            {
                control = object->control;
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = (T*)object;
#endif
        }
        
        WeakRef(const WeakRef& copy)
        {
            control = copy.control;
            if (control)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = copy.ptr;
#endif
        }
        
        WeakRef& operator=(const WeakRef& copy)
        {
            if (control)
            {
                control->ReleaseWeakRef();
            }
            control = copy.control;
            if (control)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = copy.ptr;
#endif
            return *this;
        }
        
        WeakRef& operator=(T* newObject)
        {
            if (control)
            {
                control->ReleaseWeakRef();
            }
            control = nullptr;
            if (newObject != nullptr)
            {
                control = newObject->control;
            }
            if (control != nullptr)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = newObject;
#endif
            return *this;
        }
        
        template<class U> requires TIsBaseClassOf<T, U>::Value and (not std::is_same_v<T, U>)
        WeakRef& operator=(U* newObject)
        {
            if (control)
            {
                control->ReleaseWeakRef();
            }
            control = nullptr;
            if (newObject != nullptr)
            {
                control = newObject->control;
            }
            if (control != nullptr)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = (T*)newObject;
#endif
            return *this;
        }
        
        template<class U> requires TIsBaseClassOf<T, U>::Value
        WeakRef(const WeakRef<U>& copy)
        {
            control = copy.control;
            if (control)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = copy.ptr;
#endif
        }
        
        WeakRef(const Ref<T>& from)
        {
            control = from.control;
            if (control)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = from.ptr;
#endif
        }
        
        WeakRef& operator=(const Ref<T>& from)
        {
            if (control)
            {
                control->ReleaseWeakRef();
            }
            control = from.control;
            if (control)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = from.ptr;
#endif
            return *this;
        }

        template<class U> requires TIsBaseClassOf<T, U>::Value and (not std::is_same_v<T, U>)
        WeakRef(const Ref<U>& from)
        {
            control = from.control;
            if (control)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = from.ptr;
#endif
        }

        template<class U> requires TIsBaseClassOf<T, U>::Value and (not std::is_same_v<T, U>)
        WeakRef& operator=(const Ref<U>& from)
        {
            if (control)
            {
                control->ReleaseWeakRef();
            }
            control = from.control;
            if (control)
            {
                control->AddWeakRef();
            }
#if CE_BUILD_DEBUG
            ptr = from.ptr;
#endif
            return *this;
        }
        
        WeakRef(WeakRef&& move)
        {
            control = move.control;
            move.control = nullptr;
#if CE_BUILD_DEBUG
            ptr = move.ptr;
            move.ptr = nullptr;
#endif
        }

        /*template<class U> requires TIsBaseClassOf<Object, U>::Value and (not std::is_same_v<T, U>)
        explicit operator WeakRef<U>() const
        {
            return WeakRef<U>((U*)Get());
        }*/
        
        ~WeakRef()
        {
            if (control)
            {
                control->ReleaseWeakRef();
                control = nullptr;
            }
#if CE_BUILD_DEBUG
            ptr = nullptr;
#endif
        }
        
        T* Get() const
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

        bool IsValid() const
        {
            return !IsNull();
        }

        Ref<T> Lock() const
        {
            return Ref<T>(Get());
        }
        
        T& operator*() const
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
        
        inline bool operator!() const
        {
            return Get() == nullptr;
        }
        
        inline operator bool() const
        {
            return Get() != nullptr;
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
        
        inline bool operator==(const WeakRef& rhs) const
        {
            return control == rhs.control;
        }
        
        inline bool operator!=(const WeakRef& rhs) const
        {
            return !operator==(rhs);
        }

        inline bool operator==(const Ref<T>& rhs) const
        {
            return control == rhs.control;
        }

        inline bool operator!=(const Ref<T>& rhs) const
        {
            return !operator==(rhs);
        }

        template<typename U> requires TIsBaseClassOf<T, U>::Value or TIsBaseClassOf<U, T>::Value
        bool operator==(const WeakRef<U>& rhs) const
        {
            return control == rhs.control;
        }

        template<typename U> requires TIsBaseClassOf<T, U>::Value or TIsBaseClassOf<U, T>::Value
        bool operator!=(const WeakRef<U>& rhs) const
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

        SIZE_T GetHash() const
        {
            return (SIZE_T)control;
        }
        
    private:

        template<typename U>
        friend class WeakRef;

        template<typename U>
        friend class Ref;

#if CE_BUILD_DEBUG
        T* ptr = nullptr;
#endif
        Internal::RefCountControl* control = nullptr;
    };

    template <typename T>
    bool Ref<T>::operator==(const WeakRef<T>& rhs) const
    {
        return control == rhs.control;
    }

    template <typename T>
    template<typename U> requires TIsBaseClassOf<T, U>::Value or TIsBaseClassOf<U, T>::Value
    bool Ref<T>::operator==(const WeakRef<U>& rhs) const
    {
        return control == rhs.control;
    }

} // namespace CE
