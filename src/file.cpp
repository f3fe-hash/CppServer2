#include "file.hpp"

FileCache::FileCache(uint32_t cacheSize)
{
    this->cache     = new uint8_t[cacheSize];
    this->cacheSize = cacheSize;
    this->offset    = 0;

    updater = std::thread([this]()
    {
        while (this->running)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            this->update();
        }
    });
}

FileCache::~FileCache()
{
    updater.join();
    delete[] this->cache;
}

void FileCache::update()
{
    std::lock_guard<std::mutex> cacheLock(this->cacheMutex);

    // Wipe cache
    this->offset = 0;
    this->fileMap.clear();

    for (const std::string& file : cachedFiles)
    {
        std::ifstream f(file);
        if (!f)
        {
            _log << "Failed to open " << file << std::endl;
            continue;
        }

        std::stringstream buffer;
        buffer << f.rdbuf();
        std::string data = buffer.str();

        uint32_t size = data.size();
        if (size > this->cacheSize - this->offset)
            continue;

        // Lock the file for writing
        {
            std::unique_lock<std::shared_mutex> lock(fileLocks[file]);

            std::memcpy(this->cache + this->offset, data.data(), size);
            fileMap[file] = {this->offset, size};
            this->offset += size;
        }

        std::this_thread::yield();
    }
}

std::pair<std::string, uint32_t> FileCache::readFile(const std::string& filename)
{
    {
        std::shared_lock<std::shared_mutex> lock(fileLocks[filename]);

        auto it = fileMap.find(filename);
        if (it != fileMap.end())
        {
            const FileDescriptor& desc = it->second;
            return {std::string(reinterpret_cast<char*>(this->cache + desc.offset), desc.size), desc.size};
        }
    }

    // Not in cache, read from disk
    std::ifstream file(filename);
    if (!file)
    {
        _log << "Failed to open " << filename << std::endl;
        return {"", 0};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string data = buffer.str();

    return {data, data.size()};
}
