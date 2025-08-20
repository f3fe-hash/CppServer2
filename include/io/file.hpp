#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <condition_variable>

#include "utils/utils.hpp"
#include "utils/log.hpp"

namespace fs = std::filesystem;

typedef struct
{
    std::string data;
    std::mutex data_mutex;
    std::chrono::system_clock::time_point last_loaded;
    fs::file_time_type last_modified;
} FileCacheEntry;

class FileCache
{
    std::unordered_map<std::string, std::shared_ptr<FileCacheEntry>> cache_map;
    std::shared_mutex map_mutex;

    int refresh_interval;
    std::atomic<bool> stop_flag;
    std::thread refresher_thread;
    std::condition_variable_any cv;

    std::shared_ptr<FileCacheEntry> createEntry(const std::string& path);

    void refreshLoop();
    void refreshAll();
    void loadFile(const std::string& path, FileCacheEntry& entry);

public:
    FileCache(int refresh_interval_seconds = 10);
    ~FileCache();

    std::string readFile(const std::string& path);
};
