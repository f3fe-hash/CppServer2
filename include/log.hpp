#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "utils.hpp"

class Log
{
    std::ofstream file;

public:
    Log(const std::string& filename);
    ~Log();

    size_t log(const std::string& message);

    template <typename T>
    Log& operator<<(const T& value)
    {
        std::ostringstream oss;
        oss << value;

        std::string output = oss.str();
        log(output);

        return *this;
    }

    /* Support manipulators like std::endl. */
    Log& operator<<(std::ostream& (*manip)(std::ostream&));
};

extern Log _log;

#endif
