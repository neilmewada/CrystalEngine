#include "CoreMinimal.h"

#include <chrono>
#include <ctime>
#include <spdlog/fmt/chrono.h>

namespace CE
{
    DateTime::DateTime()
    {
        auto now = std::chrono::system_clock::now();

        time_t timeUtc = std::chrono::system_clock::to_time_t(now);

        timeInfo = *std::localtime(&timeUtc);
    }

    DateTime::~DateTime()
    {

    }

    DateTime::DateTime(fs::file_time_type fileTime)
    {
        //const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
        const auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(fileTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        const auto time = std::chrono::system_clock::to_time_t(systemTime);
        timeInfo = *std::localtime(&time);
    }

    DateTime DateTime::Now()
    {
        auto now = std::chrono::system_clock::now();

        time_t timeUtc = std::chrono::system_clock::to_time_t(now);

        DateTime result{};
        result.timeInfo = *std::localtime(&timeUtc);
        
        return result;
    }

    DateTime DateTime::UtcNow()
    {
        auto now = std::chrono::system_clock::now();

        time_t timeUtc = std::chrono::system_clock::to_time_t(now);

        DateTime result{};
        result.timeInfo = *std::gmtime(&timeUtc);

        return result;
    }

    void DateTime::AddSeconds(int seconds)
    {
        if (seconds == 0)
            return;

        timeInfo.tm_sec += seconds;
        std::mktime(&timeInfo);
    }

    void DateTime::AddMinutes(int minutes)
    {
        if (minutes == 0)
            return;

        timeInfo.tm_min += minutes;
        std::mktime(&timeInfo);
    }

    void DateTime::AddHours(int hours)
    {
        if (hours == 0)
            return;

        timeInfo.tm_hour += hours;
        std::mktime(&timeInfo);
    }

    void DateTime::AddDays(int days)
    {
        if (days == 0)
            return;

        timeInfo.tm_mday += days;
        std::mktime(&timeInfo);
    }

    void DateTime::AddMonths(int months)
    {
        if (months == 0)
            return;

        timeInfo.tm_mon += months;
        std::mktime(&timeInfo);
    }

    void DateTime::AddYears(int years)
    {
        if (years == 0)
            return;

        timeInfo.tm_year += years;
        std::mktime(&timeInfo);
    }

    u64 DateTime::ToNumber() const
    {
        PackedDateTime packed{};
        packed.year = Year();
        packed.month = Month();
        packed.day = Day();
        packed.hour = Hour();
        packed.minute = Minute();
        packed.second = Second();

        return packed.finalValue;
    }

    DateTime DateTime::FromNumber(u64 number)
    {
        PackedDateTime packed{};
        packed.finalValue = number;

        DateTime result{};
        result.timeInfo.tm_year = (int)packed.year - 1900;
        result.timeInfo.tm_mon = (int)packed.month - 1;
        result.timeInfo.tm_mday = (int)packed.day;
        result.timeInfo.tm_hour = (int)packed.hour;
        result.timeInfo.tm_min = (int)packed.minute;
        result.timeInfo.tm_sec = (int)packed.second;

        return result;
    }

    String DateTime::ToString() const
    {
        constexpr const char* defaultFormat = "{:%Y-%m-%d %H:%M:%S}";
        return fmt::format(defaultFormat, timeInfo);
    }

    DateTime DateTime::Parse(const String& string)
    {
        constexpr const char* defaultFormat = "%Y-%m-%d %H:%M:%S";
        DateTime result = {};

        std::istringstream iss(string.GetCString());
        iss >> std::get_time(&result.timeInfo, defaultFormat);

        return result;
    }

    bool DateTime::operator<(const DateTime& rhs) const
    {
        return std::mktime(const_cast<std::tm*>(&timeInfo)) < std::mktime(const_cast<std::tm*>(&rhs.timeInfo));
    }

} // namespace CE
