#pragma once

#define INTRUSIVE_IMPL\
    template<typename T>\
    friend struct IntrusivePtrCountPolicy;

namespace CE
{

	//
	//  IntrusivePtr.hpp
	//
	//  Copyright (c) 2001, 2002 Peter Dimov
	//
	// Distributed under the Boost Software License, Version 1.0. (See
	// accompanying file LICENSE_1_0.txt or copy at
	// http://www.boost.org/LICENSE_1_0.txt)
	//
	//  See http://www.boost.org/libs/smart_ptr/IntrusivePtr.html for documentation.
	//
    
	struct IntrusiveDefaultDeleter
	{
		template<typename U>
		void operator()(U* p) const
		{
			delete p;
		}
	};

	template<typename T>
	struct IntrusivePtrCountPolicy
	{
		static inline void AddRef(T* p) { p->_AddRef(); }
		static inline void ReleaseRef(T* p) { p->_ReleaseRef(); }
	};

	template<typename RefCntType, typename Deleter = IntrusiveDefaultDeleter>
	class IntrusiveBaseRefCnt
	{
	public:

		virtual ~IntrusiveBaseRefCnt()
		{

		}

		inline u32 GetRefCount() const
		{
			return refCount;
		}

	protected:

		IntrusiveBaseRefCnt() = default;
		IntrusiveBaseRefCnt(Deleter deleter) : deleter(deleter)
		{}

		IntrusiveBaseRefCnt(const IntrusiveBaseRefCnt&) = delete;
		IntrusiveBaseRefCnt(IntrusiveBaseRefCnt&&) = delete;

	private:

		void _AddRef() const
		{
			++refCount;
		}

		void _ReleaseRef() const
		{
			const int refCnt = static_cast<int>(--this->refCount);
			if (refCnt == 0)
			{
				deleter(this);
			}
		}

		mutable RefCntType refCount = {};
		Deleter deleter = {};

		template<typename T>
		friend struct IntrusivePtrCountPolicy;
	};

	template<class T>
	class IntrusivePtr final
	{
	private:
		template<typename U>
		friend class IntrusivePtr;

		typedef IntrusivePtr ThisType;
		typedef IntrusivePtrCountPolicy<T> CountPolicy;

	public:

		IntrusivePtr() : ptr(0)
		{

		}

		IntrusivePtr(T* p) : ptr(p)
		{
			if (ptr != 0)
			{
				CountPolicy::AddRef(ptr);
			}
		}

		template<class U> requires std::is_convertible<U*, T*>::value
		IntrusivePtr(const IntrusivePtr<U>& rhs) : ptr(rhs.Get())
		{
			if (ptr != 0)
			{
				CountPolicy::AddRef(ptr);
			}
		}

		IntrusivePtr(const IntrusivePtr& rhs) : ptr(rhs.ptr)
		{
			if (ptr != 0)
			{
				CountPolicy::AddRef(ptr);
			}
		}

		~IntrusivePtr()
		{
			if (ptr != 0)
			{
				CountPolicy::ReleaseRef(ptr);
			}
		}

		IntrusivePtr(IntrusivePtr&& rhs) : ptr(rhs.ptr)
		{
			rhs.ptr = 0;
		}

		IntrusivePtr& operator=(IntrusivePtr&& rhs)
		{
			ThisType(static_cast<IntrusivePtr&&>(rhs)).Swap(*this);
			return *this;
		}

		IntrusivePtr& operator=(IntrusivePtr const& rhs)
		{
			ThisType(rhs).Swap(*this);
			return *this;
		}

		IntrusivePtr& operator=(T* rhs)
		{
			ThisType(rhs).Swap(*this);
			return *this;
		}

		inline void Reset()
		{
			ThisType().Swap(*this);
		}

		inline void Reset(T* rhs)
		{
			ThisType(rhs).Swap(*this);
		}

		inline T* Get() const
		{
			return ptr;
		}

		inline operator T*() const
		{
			return ptr;
		}

		inline T& operator*() const
		{
			return *ptr;
		}

		inline T* operator->() const
		{
			return ptr;
		}

		inline bool operator!() const 
		{ 
			return ptr == 0; 
		}

		inline operator bool() const
		{
			return ptr != 0;
		}

		inline bool operator==(const IntrusivePtr& rhs) const
		{
			return ptr == rhs.ptr;
		}

		inline bool operator!=(const IntrusivePtr& rhs) const
		{
			return ptr != rhs.ptr;
		}

		inline bool operator==(const T* rhs) const
		{
			return ptr == rhs;
		}

		inline bool operator!=(const T* rhs) const
		{
			return ptr != rhs;
		}

		inline void Swap(IntrusivePtr& rhs)
		{
			T* temp = ptr;
			ptr = rhs.ptr;
			rhs.ptr = temp;
		}

	private:
		T* ptr = nullptr;
	};

	using IntrusiveBase = IntrusiveBaseRefCnt<std::atomic<int>>;

	template<typename T>
	using Ptr = IntrusivePtr<T>;

} // namespace CE
