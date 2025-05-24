#pragma once

#include <mutex>
#include <shared_mutex>
#include <semaphore>
#include <chrono>
#include <thread>
#include <map>
#include <condition_variable>

namespace CE
{
    
    class CORE_API Mutex
    {
    public:
        Mutex();
        ~Mutex();

        CE_INLINE void Lock() { mut.lock(); }
        CE_INLINE void Unlock() { mut.unlock(); }
        CE_INLINE bool TryLock() { return mut.try_lock(); }

        void lock() { mut.lock(); }
        void unlock() { mut.unlock(); }
        bool try_lock() { return mut.try_lock(); }

		inline operator std::mutex&()
		{
			return mut;
		}

    private:
        std::mutex mut{};
    };

	class CORE_API SharedMutex
	{
	public:
		SharedMutex();
		~SharedMutex();

		CE_INLINE void Lock() { mut.lock(); }
		CE_INLINE void Unlock() { mut.unlock(); }
		CE_INLINE bool TryLock() { return mut.try_lock(); }

		void lock() { mut.lock(); }
		void unlock() { mut.unlock(); }
		bool try_lock() { return mut.try_lock(); }

		inline operator std::shared_mutex& ()
		{
			return mut;
		}

	private:
		std::shared_mutex mut{};
	};

	class CORE_API RecursiveMutex
	{
	public:
		RecursiveMutex() {}
		~RecursiveMutex() {}

		CE_INLINE void Lock() { mut.lock(); }
		CE_INLINE void Unlock() { mut.unlock(); }
		CE_INLINE bool TryLock() { return mut.try_lock(); }

		void lock() { mut.lock(); }
		void unlock() { mut.unlock(); }
		bool try_lock() { return mut.try_lock(); }

		inline operator std::recursive_mutex& ()
		{
			return mut;
		}

	private:
		std::recursive_mutex mut{};
	};

	class CORE_API SharedRecursiveMutex
    {
    public:

        SharedRecursiveMutex() :
            m_mtx{}, m_exclusive_thread_id{}, m_exclusive_count{ 0 }, m_shared_locks{}
        {}

	    void lock() { Lock(); }
	    void unlock() { Unlock(); }
	    bool try_lock() { return TryLock(); }

        void Lock();
        bool TryLock();
        void Unlock();

        void LockShared();
        bool TryLockShared();
        void UnlockShared();

        SharedRecursiveMutex(const SharedRecursiveMutex&) = delete;
        SharedRecursiveMutex& operator=(const SharedRecursiveMutex&) = delete;

    private:

        inline bool is_exclusive_locked()
        {
            return m_exclusive_count > 0;
        }

        inline bool is_shared_locked()
        {
            return m_shared_locks.size() > 0;
        }

        inline bool can_exclusively_lock()
        {
            return can_start_exclusive_lock() || can_increment_exclusive_lock();
        }

        inline bool can_start_exclusive_lock()
        {
            return !is_exclusive_locked() && (!is_shared_locked() || is_shared_locked_only_on_this_thread());
        }

        inline bool can_increment_exclusive_lock()
        {
            return is_exclusive_locked_on_this_thread();
        }

        inline bool can_lock_shared()
        {
            return !is_exclusive_locked() || is_exclusive_locked_on_this_thread();
        }

        inline bool is_shared_locked_only_on_this_thread()
        {
            return is_shared_locked_only_on_thread(std::this_thread::get_id());
        }

        inline bool is_shared_locked_only_on_thread(std::thread::id id)
        {
            return m_shared_locks.size() == 1 && m_shared_locks.find(id) != m_shared_locks.end();
        }

        inline bool is_exclusive_locked_on_this_thread()
        {
            return is_exclusive_locked_on_thread(std::this_thread::get_id());
        }

        inline bool is_exclusive_locked_on_thread(std::thread::id id)
        {
            return m_exclusive_count > 0 && m_exclusive_thread_id == id;
        }

        inline void start_exclusive_lock()
        {
            m_exclusive_thread_id = std::this_thread::get_id();
            m_exclusive_count++;
        }

        inline void increment_exclusive_lock()
        {
            m_exclusive_count++;
        }

        inline void decrement_exclusive_lock()
        {
            if (m_exclusive_count == 0)
            {
                throw std::logic_error("Not exclusively locked, cannot exclusively unlock");
            }
            if (m_exclusive_thread_id == std::this_thread::get_id())
            {
                m_exclusive_count--;
            }
            else
            {
                throw std::logic_error("Calling exclusively unlock from the wrong thread");
            }
        }

        inline void increment_shared_lock()
        {
            increment_shared_lock(std::this_thread::get_id());
        }

        inline void increment_shared_lock(std::thread::id id)
        {
            if (m_shared_locks.find(id) == m_shared_locks.end())
            {
                m_shared_locks[id] = 1;
            }
            else
            {
                m_shared_locks[id] += 1;
            }
        }

        inline void decrement_shared_lock()
        {
            decrement_shared_lock(std::this_thread::get_id());
        }

        inline void decrement_shared_lock(std::thread::id id)
        {
            if (m_shared_locks.size() == 0)
            {
                throw std::logic_error("Not shared locked, cannot shared unlock");
            }
            if (m_shared_locks.find(id) == m_shared_locks.end())
            {
                throw std::logic_error("Calling shared unlock from the wrong thread");
            }
            else
            {
                if (m_shared_locks[id] == 1)
                {
                    m_shared_locks.erase(id);
                }
                else
                {
                    m_shared_locks[id] -= 1;
                }
            }
        }

        std::mutex m_mtx;
        std::thread::id m_exclusive_thread_id;
        size_t m_exclusive_count;
        std::map<std::thread::id, size_t> m_shared_locks;
        std::condition_variable m_cond_var;
    };
    
    template<class TMutex>
    struct LockGuard
    {
    public:
        LockGuard(TMutex& mutex) : mutex(mutex)
        {
            mutex.Lock();
        }

        ~LockGuard() noexcept
        {
            mutex.Unlock();
        }

        LockGuard(const LockGuard& copy) = delete;
        LockGuard operator=(const LockGuard& copy) = delete;

    private:
		TMutex& mutex;
    };

	template<typename T>
	using Atomic = std::atomic<T>;

	using BinarySemaphore = std::binary_semaphore;

	using CountingSemaphore = std::counting_semaphore<>;

} // namespace CE
