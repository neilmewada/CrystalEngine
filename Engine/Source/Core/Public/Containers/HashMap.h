#pragma once

#include "Algorithm/Hash.h"

#include <unordered_map>

#include "unordered_dense.h"

namespace CE
{
    class Name;

    template<typename KeyType, typename ValueType>
    class Pair
    {
    public:

        constexpr Pair() : first({}), second({})
        {

        }

        constexpr Pair(KeyType key, ValueType value) : first(key), second(value)
        {

        }

        KeyType first;
        ValueType second;

        SIZE_T GetHash() const
        {
            SIZE_T hash = CE::GetHash<KeyType>(first);
            CombineHash(hash, second);
            return hash;
        }

        bool operator==(const Pair& rhs) const
        {
            return GetHash() == rhs.GetHash();
        }

        bool operator!=(const Pair& rhs) const
        {
            return !operator==(rhs);
        }
    };
    
    template<typename KeyType = CE::Name, typename ValueType = CE::Variant>
    class HashMap
    {
    public:
        HashMap() : Impl({})
        {
            
        }
        
        HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list) : Impl(list)
        {
            
        }

        ~HashMap()
        {

        }

        inline SIZE_T GetSize() const
        {
            return Impl.size();
        }

		inline bool IsEmpty() const
		{
			return Impl.empty();
		}

        inline SIZE_T GetCount(const KeyType& key) const
        {
            return Impl.count(key);
        }

        inline bool KeyExists(const KeyType& key) const
        {
            return Impl.contains(key);
        }

        inline auto Find(const KeyType& key)
        {
            return Impl.find(key);
		}

        inline auto Begin()
        {
            return Impl.begin();
        }

        inline auto End()
        {
            return Impl.end();
        }

        inline auto Begin() const
        {
            return Impl.begin();
		}

        inline auto End() const
        {
            return Impl.end();
        }

        inline ValueType& operator[](const KeyType& key)
        {
            return Impl[key];
        }
        
        inline ValueType& Get(const KeyType& key)
        {
            return Impl.at(key);
        }
        
        inline const ValueType& Get(const KeyType& key) const
        {
            return Impl.at(key);
        }

		inline void Add(const KeyType& key, const ValueType& value)
		{
			Impl.insert({ key, value });
		}

        inline void Add(const Pair<KeyType, ValueType>& pair)
        {
            Impl.insert({ pair.first, pair.second });
        }

		inline void AddRange(const HashMap<KeyType, ValueType>& from)
		{
			for (const auto& [key, value] : from)
			{
				Add({ key, value });
			}
		}

        template<typename... Args>
        auto Emplace(Args... args)
        {
            return Impl.emplace(args...);
        }

        void Remove(const KeyType& key)
        {
            Impl.erase(key);
        }
        
        void Clear()
        {
            Impl.clear();
        }

        inline auto begin() { return Impl.begin(); }
        inline auto end() { return Impl.end(); }

        inline const auto begin() const { return Impl.begin(); }
        inline const auto end() const { return Impl.end(); }

    private:
        ankerl::unordered_dense::map<KeyType, ValueType, HashFunc<KeyType>> Impl;
    };

} // namespace CE

