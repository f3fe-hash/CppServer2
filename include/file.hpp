#ifndef __FILE_HPP__
#define __FILE_HPP__

#include <unordered_map>
#include <thread> // For std::this_thread

#include <string>
#include <cstring>

#include <iostream>
#include <fstream>

#include "types.h"
#include "utils.hpp"

using FileDescriptor = struct FileDescriptor
{
    uint32_t offset;
    uint32_t size;
};

class FileCache
{
    std::unordered_map<std::string, FileDescriptor> fileMap;

    uint8_t* cache;
    uint32_t cacheSize;
    uint32_t offset;
public:
    FileCache(uint32_t cacheSize);
    ~FileCache();

    std::string readFile(std::string fileName);
};

#endif