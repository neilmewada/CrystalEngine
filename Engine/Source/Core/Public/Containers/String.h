#pragma once

#include "Algorithm/Hash.h"
#include "Containers/Array.h"

#include "spdlog/fmt/fmt.h"

#ifndef STRING_BUFFER_SIZE
#define STRING_BUFFER_SIZE 32
#endif

#ifndef STRING_BUFFER_GROW_COUNT
#define STRING_BUFFER_GROW_COUNT 32
#endif

#include <string>
#include <ostream>


namespace CE
{
    class StringView;
    class Stream;

    template<typename T>
    class Array;

	class CORE_API String
	{
	public:
        struct Iterator;

        String();
        String(std::string string);
        String(StringView stringView);
        explicit String(u32 reservedSize);
        String(const char* value);
        String(const char* cStringA, const char* cStringB);
        String(Iterator begin, Iterator end);

        String(String&& move) noexcept;
        String(const String& copy);
        String& operator=(const String& rhs);
        String& operator=(const std::string& rhs);
        String& operator=(const char* cString);

        // Concatenate operators
        inline String operator+(const String& other) const
        {
            String result = String(*this);
            result.Concatenate(other);
            return result;
        }

        inline String& operator+=(const String& other)
        {
            this->Concatenate(other);
            return *this;
        }
        inline String operator+(const char* cString) const
        {
            String result = String(*this);
            result.ConcatenateCString(cString);
            return result;
        }
        friend inline String operator+(const char* lhs, const String& rhs)
        {
            return String(lhs, rhs.GetCString());
        }
        inline String& operator+=(const char* cString)
        {
            this->ConcatenateCString(cString);
            return *this;
        }

        inline String& operator+(s64 integer)
        {
            this->Concatenate(integer);
            return *this;
        }
        inline String& operator+=(s64 integer)
        {
            this->Concatenate(integer);
            return *this;
        }
        inline operator std::string() const
        {
            return std::string(Buffer);
        }

        inline bool operator<(const String& other) const
        {
            return StringLength < other.StringLength;
        }
        inline bool operator==(const String& other) const
        {
            return strcmp(GetCString(), other.GetCString()) == 0;
        }
        inline bool operator!=(const String& other) const
        {
            return !(*this == other);
        }
        inline bool operator==(const char* cString) const
        {
            if (cString == GetCString())
                return true;
            if (cString == nullptr || GetCString() == nullptr)
                return false;
            return strcmp(GetCString(), cString) == 0;
        }
        inline bool operator!=(const char* CString) const
        {
            return !(*this == CString);
        }

        inline char& operator[](u32 index)
        {
            return Buffer[index];
        }
        inline char operator[](u32 index) const
        {
            return Buffer[index];
        }

        friend inline std::ostream& operator<<(std::ostream& os, const String& string)
        {
            os << string.GetCString();
            return os;
        }

        ~String();

        /*
         *  Main Functions
         */

         // Reserves space for the given number of characters in string.
        void Reserve(u32 reserveCharacterCount);

		// Warning: Internal use only! This function frees the memory used by this string.
		void Free();

        char* GetCString() const;
		char* GetData() const;
        inline u32 GetLength() const { return StringLength; }
        inline u32 GetCapacity() const { return Capacity; }

		inline char GetFirst() const { return GetLength() == 0 ? 0 : Buffer[0]; }
		inline char GetLast() const { return GetLength() == 0 ? 0 : Buffer[GetLength() - 1]; }

        inline std::string ToStdString() const { return std::string(Buffer); }

        inline void Clear()
        {
            SetCString(nullptr);
        }

        inline bool IsEmpty() const { return StringLength == 0; }
		inline bool NotEmpty() const { return StringLength > 0; }

        bool IsEmptyOrWhiteSpace() const;

		/// Returns true if lhs is less than rhs
		static bool NaturalCompare(const String& lhs, const String& rhs);
		static bool Compare(const String& lhs, const String& rhs);

        /*
         *  Iterators
         */

        struct Iterator
        {
            using iterator_category = std::contiguous_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = char;
            using pointer = char*;  // or also value_type*
            using reference = char&;  // or also value_type&

            Iterator(pointer Ptr) : Ptr(Ptr) {}

            // De-reference ops
            reference operator*() const { return *Ptr; }
            pointer operator->() { return Ptr; }

            // Increment ops
            Iterator& operator++() { Ptr++; return *this; }
            Iterator operator++(int) { Iterator temp = *this; ++(*this); return temp; }
            // Decrement ops
            Iterator& operator--() { Ptr--; return *this; }
            Iterator operator--(int) { Iterator temp = *this; --(*this); return temp; }

            friend bool operator== (const Iterator& a, const Iterator& b) { return a.Ptr == b.Ptr; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.Ptr != b.Ptr; };

        private:
            pointer Ptr;
            friend class CE::String;
        };

		struct ConstIterator
		{
			using iterator_category = std::contiguous_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = char;
			using pointer = char*;  // or also value_type*
			using reference = char&;  // or also value_type&

			ConstIterator(pointer Ptr) : Ptr(Ptr) {}

			// De-reference ops
			reference operator*() const { return *Ptr; }
			pointer operator->() { return Ptr; }

			// Increment ops
			ConstIterator& operator++() { Ptr++; return *this; }
			ConstIterator operator++(int) { ConstIterator temp = *this; ++(*this); return temp; }
			// Decrement ops
			ConstIterator& operator--() { Ptr--; return *this; }
			ConstIterator operator--(int) { ConstIterator temp = *this; --(*this); return temp; }

			friend bool operator== (const ConstIterator& a, const ConstIterator& b) { return a.Ptr == b.Ptr; };
			friend bool operator!= (const ConstIterator& a, const ConstIterator& b) { return a.Ptr != b.Ptr; };

		private:
			pointer Ptr;
			friend class CE::String;
		};

        Iterator begin() { return Iterator{ &Buffer[0] }; }
        Iterator end() { return Iterator{ &Buffer[0] + StringLength }; }

		ConstIterator cbegin() const { return ConstIterator{ &Buffer[0] }; }
		ConstIterator cend() const { return ConstIterator{ &Buffer[0] + StringLength }; }

        Iterator Begin() { return begin(); }
        Iterator End() { return end(); }

        /*
         *  Helper/Utility
         */

        template<typename NativeType>
        inline static String FromNativeType(NativeType value)
        {
            return String(std::to_string(value));
        }

	    template<typename  T>
        static auto Arg(const char* name, const T& arg)
        {
            return fmt::arg(name, arg);
        }
	    
        template<typename... Args>
        static String Format(const String& str, Args... args)
        {
            return String(fmt::vformat(str.Buffer, fmt::make_format_args(args...)));
        }

		template<typename... Args>
		static String Format(const char* cStr, Args... args)
        {
        	return String(fmt::vformat(cStr, fmt::make_format_args(args...)));
        }

	    static bool RegexMatch(const String& sourceString, const String& regex);
	    static bool RegexMatch(const String& sourceString, const char* regex);

        void Append(char c);

        void Concatenate(const String& string);
        void ConcatenateCString(const char* cString);
        void Concatenate(s64 integer);

        bool StartsWith(const String& string) const;
        bool StartsWith(const char* cString) const;

        bool EndsWith(const String& string) const;
        bool EndsWith(const char* cString) const;

        bool Contains(const String& string) const;
        bool Contains(const char* string) const;
	    bool Contains(char character) const;

        bool Search(const String& string) const;
        bool Search(const char* string) const;

        String ToLower() const;
        String ToUpper() const;

		/// To: kebab-case
		String ToKebabCase() const;
		/// To: snake_case
		String ToSnakeCase() const;
		/// To: camelCase
		String ToCamelCase() const;
		/// To: PascalCase
		String ToPascalCase() const;

        String GetSubstring(int startIndex, int length = -1) const;
        StringView GetSubstringView(int startIndex, int length = -1) const;

        StringView ToStringView() const;

        Array<String> Split(char delimiter) const;
        void Split(String delimiter, Array<String>& outArray) const;
		void Split(const Array<String>& delimiters, Array<String>& outArray) const;

		void Split(InitializerList<String> delimiters, Array<String>& outArray) const;

        String RemoveWhitespaces() const;

		String Replace(const Array<char>& charsToReplace, char replaceWith) const;

        void RemoveAt(int index);

        void InsertAt(char c, int index);

        void Remove(int startIndex, int count = -1);

		void UpdateLength();

	    // Character Utils

	    FORCE_INLINE static bool IsNumeric(char c)
	    {
	        return c >= '0' && c <= '9';
	    }

	    FORCE_INLINE static u8 CharToNumber(char c)
	    {
	        if (!IsNumeric(c))
	            return 0;
	        return (u8)(c - '0');
	    }

		FORCE_INLINE static bool IsSpecial(char c)
		{
			return (c >= 32 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
		}

	    FORCE_INLINE static bool IsAlphabet(char c)
	    {
	        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	    }

	    FORCE_INLINE static bool IsUpper(char c)
	    {
	        return c >= 'A' && c <= 'Z';
	    }

	    FORCE_INLINE static bool IsLower(char c)
	    {
	        return c >= 'a' && c <= 'z';
	    }

	    // Parsing

	    template<typename T>
		static bool TryParseValue(const String& string, T& outValue)
	    {
			if constexpr (TIsIntegralType<T>::Value)
			{
				s64 value = 0;
				if (TryParseInteger(string, value))
				{
					outValue = static_cast<T>(value);
					return true;
				}
				return false;
			}
			else if constexpr (TIsSameType<T, f32>::Value or TIsSameType<T, f64>::Value)
			{
				T value = 0;
				if (TryParseFloat(string, value))
				{
					outValue = static_cast<T>(value);
					return true;
				}
				return false;
			}
			else
			{
				return TryParse(string, outValue);
			}
	    }

		static bool TryParse(const String& string, c8& outValue);
		static bool TryParse(const String& string, c16& outValue);
	    static bool TryParse(const String& string, u8& outValue);
	    static bool TryParse(const String& string, s8& outValue);
		static bool TryParse(const String& string, u16& outValue);
		static bool TryParse(const String& string, s16& outValue);
	    static bool TryParse(const String& string, u32& outValue);
	    static bool TryParse(const String& string, s32& outValue);
        static bool TryParse(const String& string, u64& outValue);
        static bool TryParse(const String& string, s64& outValue);
	    static bool TryParse(const String& string, f32& outValue);
		static bool TryParse(const String& string, f64& outValue);
        static bool TryParse(const String& string, b8& outValue);

	    template<typename T>
	    FORCE_INLINE static T Parse(const String& string)
	    {
	        T retVal{};
	        if (!TryParseValue(string, retVal))
	            Internal_ThrowParseException("Failed to parse string");
	        return retVal;
	    }

    private:
	    friend class Stream;

        static void Internal_ThrowParseException(const char* message);
	    
        static bool TryParseInteger(String value, s64& outValue);
	    static bool TryParseFloat(String value, f32& outValue);
		static bool TryParseFloat(String value, f64& outValue);
	    
	    // Internals

        void SetCString(const char* cString);
        void CopyCString(const char* cString, u32 copyStringLength);

		char* Buffer = nullptr;
		u32 Capacity = 0; // Size of dynamic buffer in bytes

		//bool bIsUsingDynamicBuffer = false;
		u32 StringLength = 0;

        //void* impl = nullptr;
	};

    template<>
    inline SIZE_T GetHash(const String& Value)
    {
        return CalculateHash(Value.GetCString(), Value.GetLength());
    }

	template<SIZE_T Bits>
	inline String BitSet<Bits>::ToString() const
	{
		return impl.to_string();
	}
}

/// fmt user-defined Formatter for CE::String
template <> struct fmt::formatter<CE::String> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::String& string, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", string.GetCString());
    }
};

