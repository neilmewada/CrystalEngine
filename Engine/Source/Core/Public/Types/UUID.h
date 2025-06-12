#pragma once

#include "Types/CoreTypeDefs.h"

#include "Algorithm/Hash.h"
#include "Containers/String.h"
#include "Containers/StringView.h"

namespace CE
{
    CORE_API u64 GenerateRandomU64();
    CORE_API u32 GenerateRandomU32();

    /// A 64-bit unique identifier
    class CORE_API Uuid
    {
    public:
        Uuid();

        Uuid(u64 low, u64 high);
        Uuid(Hash128 hash);

        static Uuid Random();

        static Uuid New();

        static Uuid Null();
        static Uuid Zero();

        static Uuid FromString(const String& uuidString);

        bool IsNull() const;
        bool IsValid() const { return !IsNull(); }
        
        Uuid(const Uuid&) = default;
        
        bool operator==(const Uuid& other) const
        {
            return uuid[0] == other.uuid[0] && uuid[1] == other.uuid[1];
        }
        
        bool operator!=(const Uuid& other) const
        {
            return !operator==(other);
        }

        SIZE_T GetHash() const;

        String ToString() const;

        CORE_API friend Stream& operator<<(Stream& stream, const Uuid& uuid);

        CORE_API friend Stream& operator>>(Stream& stream, Uuid& uuid);
        
    private:
        u64 uuid[2] = { 0, 0 };
    };
    
} // namespace CE

/// fmt user-defined Formatter for CE::String
template <> struct fmt::formatter<CE::Uuid> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Uuid& value, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", value.ToString());
    }
};
