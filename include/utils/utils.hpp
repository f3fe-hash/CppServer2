#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <cerrno>
#include <cstring>

#include "types.h"

using namespace std::chrono;

_throw
std::string _now();

/* Handle an error. */
#define err(x) \
{ \
    std::cerr << x << " (" << _now() << ")" << "\033[?25h" << std::endl; \
    throw std::runtime_error(std::string(x) + "(" + std::strerror(errno) + ")"); \
}

#endif