#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "types.h"

using namespace std::chrono;

_throw
std::string _now();

/* Handle an error. */
#define err(x) \
{ \
    std::cerr << x << " (" << _now() << ")" << std::endl; \
    throw std::runtime_error(x); \
}

#endif