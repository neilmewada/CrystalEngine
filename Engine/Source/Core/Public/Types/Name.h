#pragma once

#include "Algorithm/Hash.h"
#include "Containers/String.h"
#include "Containers/StringView.h"

#define NAME_None CE::Name()

#define __NAME_BUNDLE_0()
#define __NAME_BUNDLE_1(Bundle) Bundle "."

#define __NAME_NAMESPACE_0()
#define __NAME_NAMESPACE_1(Namespace) #Namespace "::"

#define MAKE_NAME(Bundle, Namespace, Type)\
	CE_EXPAND(CE_CONCATENATE(__NAME_BUNDLE_, CE_ARG_COUNT(Bundle)))(Bundle) CE_EXPAND(CE_CONCATENATE(__NAME_NAMESPACE_, CE_ARG_COUNT(Namespace)))(Namespace) #Type


namespace CE
{
    class String;
    class StringView;

    template<typename ElementType>
    class Array;
    
    /*
    * Names are Case-Sensitive identifiers that offer fast comparison using hash codes.
	* Format: {BundlePath}.{ShortTypeName}
    * It follows the C++ scope pattern where the scope operator '::' is used for scope.
	* Full names include a bundle path in the front. Ex: /Engine/Core.CE::Object
    */
    class CORE_API Name
    {
    public:
        Name() : Name("")
        {}
        
        Name(String name);
        Name(const char* name);
        Name(const std::string& str) : Name(String(str))
        {}
        ~Name() = default;

        Name(const Name& copy);
        Name& operator=(const Name& copy);
		Name(Name&& move);

        template<typename T>
        Name(const char* name, const T& fieldRef) : Name(String(name))
		{}

        CE_INLINE bool IsValid() const
        {
            return hashValue != 0;
        }

        CE_INLINE SIZE_T GetHashValue() const { return hashValue; }

		const String& GetString() const;
        
		CE_INLINE const char* GetCString() const
		{
			return GetString().GetCString();
		}

        CE_INLINE bool operator==(const Name& rhs) const
        {
            return hashValue == rhs.hashValue;
        }

        CE_INLINE bool operator!=(const Name& rhs) const
        {
            return hashValue != rhs.hashValue;
        }

        void GetComponents(CE::Array<String>& components) const;
        
        String GetLastComponent() const;

        String GetParentPath() const;

    private:
        SIZE_T hashValue;
#if CE_NAME_DEBUG
		char* debugString = nullptr;
#endif

		static HashMap<SIZE_T, String> nameHashMap;
    };

    template<>
    inline SIZE_T GetHash(const Name& value)
    {
        return value.GetHashValue();
    }
    
} // namespace CE


/// fmt user-defined Formatter for CE::String
template <> struct fmt::formatter<CE::Name> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Name& name, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", name.GetString());
    }
};
