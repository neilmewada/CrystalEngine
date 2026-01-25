
#include "Types/UUID.h"

#include <random>

namespace CE
{
    static std::random_device gRandomDevice{};
    static std::mt19937_64 gRandomEngine(gRandomDevice());
    static std::uniform_int_distribution<u64> gUniformDistribution;

    static std::random_device gRandomDevice32{};
    static std::mt19937 gRandomEngine32(gRandomDevice32());
    static std::uniform_int_distribution<u32> gUniformDistribution32;

    Uuid::Uuid() : Uuid(0, 0)
    {
        
    }

    Uuid::Uuid(u64 low, u64 high)
    {
        uuid[0] = low;
        uuid[1] = high;
    }

    Uuid::Uuid(Hash128 hash)
    {
        uuid[0] = hash.low64;
        uuid[1] = hash.high64;
    }

    Uuid Uuid::Random()
    {
        return Uuid(gUniformDistribution(gRandomEngine), gUniformDistribution(gRandomEngine));
    }

    Uuid Uuid::New()
    {
        return Random();
    }

    Uuid Uuid::Null()
    {
        return Uuid(0, 0);
    }

    Uuid Uuid::Zero()
    {
        return Null();
    }

    Uuid Uuid::FromString(const String& uuidString)
    {
        std::string uuidStr = uuidString;

        if (uuidStr.size() != 36 ||
            uuidStr[8] != '-' || uuidStr[13] != '-' || uuidStr[18] != '-' || uuidStr[23] != '-') {
            return Uuid::Zero(); // Invalid format
        }

        Uuid result = Uuid::Zero();

        try {
            // Parse each section of the UUID
            uint32_t part1 = std::stoul(uuidStr.substr(0, 8), nullptr, 16);
            uint16_t part2 = std::stoul(uuidStr.substr(9, 4), nullptr, 16);
            uint16_t part3 = std::stoul(uuidStr.substr(14, 4), nullptr, 16);
            uint16_t part4 = std::stoul(uuidStr.substr(19, 4), nullptr, 16);
            uint64_t part5 = std::stoull(uuidStr.substr(24, 12), nullptr, 16);

            // Combine parts into data[0] and data[1]
            result.uuid[0] = (static_cast<uint64_t>(part1) << 32) |
                (static_cast<uint64_t>(part2) << 16) |
                part3;
            result.uuid[1] = (static_cast<uint64_t>(part4) << 48) | part5;

            return result; // Successful conversion
        }
        catch (...) {
            return Uuid::Zero(); // Conversion failed due to invalid hex values
        }
    }

    bool Uuid::IsNull() const
    {
        return uuid[0] == 0 && uuid[1] == 0;
    }

    CORE_API u64 GenerateRandomU64()
    {
        return gUniformDistribution(gRandomEngine);
    }

    CORE_API u32 GenerateRandomU32()
    {
        return gUniformDistribution32(gRandomEngine32);
    }

    SIZE_T Uuid::GetHash() const
    {
        return GetCombinedHash(uuid[0], uuid[1]);
    }

    String Uuid::ToString() const
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0')
            << std::setw(8) << (static_cast<uint32_t>(uuid[0] >> 32)) << "-"
            << std::setw(4) << (static_cast<uint16_t>(uuid[0] >> 16)) << "-"
            << std::setw(4) << (static_cast<uint16_t>(uuid[0])) << "-"
            << std::setw(4) << (static_cast<uint16_t>(uuid[1] >> 48)) << "-"
            << std::setw(12) << (uuid[1] & 0x0000FFFFFFFFFFFFULL);
        return ss.str();
    }


    CORE_API Stream& operator<<(Stream& stream, const Uuid& uuid)
    {
        stream << uuid.uuid[0];
        stream << uuid.uuid[1];
        return stream;
    }

    CORE_API Stream& operator>>(Stream& stream, Uuid& uuid)
    {
        stream >> uuid.uuid[0];
        stream >> uuid.uuid[1];
        return stream;
    }

} // namespace CE
