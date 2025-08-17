#include "log.hpp"
#include <sys/stat.h>   // mkdir
#include <sys/types.h>  // mkdir
#include <cstring>      // strerror

Log::Log(const std::string& filename)
{
    this->canLog = true;

    // Manually create logs/ directory (portable to POSIX)
    struct stat st;
    if (stat("logs", &st) != 0)
    {
        if (mkdir("logs", 0755) != 0 && errno != EEXIST)
            err(std::string("Failed to create logs/ directory: ") + std::strerror(errno))
    }

    // Retry for 1 min
    static constexpr const int MAX_ATTEMPTS = 12;
    const std::string path = "logs/" + filename;

    for (int att = 0; att < MAX_ATTEMPTS; ++att)
    {
        file.open(path, std::ios::out | std::ios::app);
        if (file.is_open())
            return;

        std::cerr << "Failed to open log file: " << path << " (attempt #" << att + 1 << ")\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    std::cerr << "Log file could not be opened after " << MAX_ATTEMPTS << " attempts. Logging disabled.\n";
    this->canLog = false;
}

Log::~Log()
{
    if (file.is_open())
        file.close();
}

size_t Log::log(const std::string& message)
{
    if (_likely(this->canLog.load() == true)) file << message;
    std::cout << message;
    return message.size();
}

// Support manipulators like std::endl
Log& Log::operator<<(std::ostream& (*manip)(std::ostream&))
{
    if (_likely(this->canLog.load())) manip(file);
    manip(std::cout);
    return *this;
}
