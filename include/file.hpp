#ifndef __FILE_HPP__
#define __FILE_HPP__

#include <unordered_map>
#include <vector>

#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>

#include <string>
#include <cstring>

#include <iostream>
#include <fstream>

#include "types.h"
#include "utils.hpp"
#include "log.hpp"

using FileDescriptor = struct FileDescriptor
{
    uint32_t offset;
    uint32_t size;
};

class FileCache
{
private:
    std::unordered_map<std::string, FileDescriptor> fileMap;
    std::unordered_map<std::string, std::shared_mutex> fileLocks;
    std::mutex lockMapMutex;

    std::vector<std::string> cachedFiles;
    std::thread updater;

    bool running;

    uint8_t* cache;
    uint32_t cacheSize;
    uint32_t offset;

    std::mutex cacheMutex;

public:
    FileCache(uint32_t cacheSize);
    ~FileCache();

    void update();
    std::pair<std::string, uint32_t> readFile(const std::string& fileName);
};

#endif