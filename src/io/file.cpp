#include "io/file.hpp"

FileCache::FileCache(int refresh_interval_seconds)
    : refresh_interval(refresh_interval_seconds), stop_flag(false)
{
    refresher_thread = std::thread(&FileCache::refreshLoop, this);
}

FileCache::~FileCache()
{
    stop_flag = true;
    cv.notify_all();
    if (refresher_thread.joinable())
        refresher_thread.join();
}

std::string FileCache::readFile(const std::string& path)
{
    std::shared_ptr<FileCacheEntry> entry = createEntry(path);
    std::lock_guard<std::mutex> lock(entry->data_mutex);
    return entry->data;
}

std::shared_ptr<FileCacheEntry> FileCache::createEntry(const std::string& path)
{
    {
        std::shared_lock lock(map_mutex);
        auto it = cache_map.find(path);
        if (_likely(it != cache_map.end()))
            return it->second;
    }

    auto entry = std::make_shared<FileCacheEntry>();
    loadFile(path, *entry);

    {
        std::unique_lock lock(map_mutex);
        cache_map[path] = entry;
    }

    return entry;
}

void FileCache::refreshLoop()
{
    while (!stop_flag)
    {
        std::this_thread::sleep_for(std::chrono::seconds(refresh_interval));
        refreshAll();
    }
}

void FileCache::refreshAll()
{
    std::shared_lock lock(map_mutex);
    for (auto& [path, entry] : cache_map)
    {
        try
        {
            auto current_mtime = fs::last_write_time(path);
            std::lock_guard<std::mutex> data_lock(entry->data_mutex);

            if (entry->last_modified != current_mtime)
                loadFile(path, *entry);
        }
        catch (const std::exception& e)
        {
            _log << "Cannot read file " << path << std::endl;
        }
    }
}

void FileCache::loadFile(const std::string& path, FileCacheEntry& entry)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return;

    std::ostringstream ss;
    ss << file.rdbuf();
    entry.data = ss.str();
    entry.last_loaded = std::chrono::system_clock::now();
    entry.last_modified = fs::last_write_time(path);
}