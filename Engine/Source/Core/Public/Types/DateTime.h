#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace CE
{

    class CORE_API DateTime final
    {
    public:

        DateTime();
        ~DateTime();

        DateTime(fs::file_time_type fileTime);

        static DateTime Now();

        static DateTime UtcNow();

        inline int Second() const { return timeInfo.tm_sec; }
        inline int Minute() const { return timeInfo.tm_min; }
        inline int Hour() const { return timeInfo.tm_hour; }
        inline int Day() const { return timeInfo.tm_mday; }
        inline int Month() const { return timeInfo.tm_mon + 1; }
        inline int Year() const { return timeInfo.tm_year + 1900; }

        inline bool operator==(const DateTime& rhs) const
        {
            return Second() == rhs.Second() && Minute() == rhs.Minute() && Hour() == rhs.Hour() &&
                Day() == rhs.Day() && Month() == rhs.Month() && Year() == rhs.Year();
        }

        inline bool operator!=(const DateTime& rhs) const
        {
            return !(*this == rhs);
        }
        
        void AddSeconds(int seconds);
        void AddMinutes(int minutes);
        void AddHours(int hours);
        void AddDays(int days);
        void AddMonths(int months);
        void AddYears(int years);

        u64 ToNumber() const;
        static DateTime FromNumber(u64 number);

        String ToString() const;
        static DateTime Parse(const String& input);

        bool operator<(const DateTime& rhs) const;

    private:

        struct PackedDateTime
        {
            union {
                struct {
                    u64 year : 16;   // 16 bits for year (up to 65535)
                    u64 month : 4;    // 4 bits for month (up to 12)
                    u64 day : 5;    // 5 bits for day (up to 31)
                    u64 hour : 5;    // 5 bits for hour (up to 23)
                    u64 minute : 6;    // 6 bits for minute (up to 60)
                    u64 second : 6;    // 6 bits for second (up to 60)
                };

                u64 finalValue = 0;
            };
        };

        std::tm timeInfo{};
    };
    
} // namespace CE
