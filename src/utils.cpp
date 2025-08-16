#include "utils.hpp"

std::string _now()
{
    // Get current time_point
    auto now = system_clock::now();

    // Extract milliseconds part
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // Convert to time_t for calendar time (seconds precision)
    std::time_t t = system_clock::to_time_t(now);

    // Convert to local time (struct tm)
    std::tm tm;
    localtime_r(&t, &tm);

    // Format the time into a string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}