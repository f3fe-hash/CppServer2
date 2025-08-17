#include "log.hpp"
#include <sys/stat.h>   // mkdir
#include <sys/types.h>  // mkdir
#include <cstring>      // strerror

Log::Log(const std::string& filename)
{
    // Manually create logs/ directory (portable to POSIX)
    struct stat st;
    if (stat("logs", &st) != 0)
    {
        if (mkdir("logs", 0755) != 0 && errno != EEXIST)
            err("Failed to create logs/ directory")
    }

    file.open("logs/" + filename, std::ios::out | std::ios::app);
    if (!file.is_open())
        err("Failed to open log file: " + filename)
}

Log::~Log()
{
    if (file.is_open())
        file.close();
}

size_t Log::log(const std::string& message)
{
    file << message;
    std::cout << message;
    return message.size();
}

// Support manipulators like std::endl
Log& Log::operator<<(std::ostream& (*manip)(std::ostream&))
{
    manip(file);
    manip(std::cout);
    return *this;
}
