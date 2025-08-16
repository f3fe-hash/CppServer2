#include "file.hpp"

FileCache::FileCache(uint32_t cacheSize)
{
    this->cache     = new uint8_t[cacheSize];
    this->cacheSize = cacheSize;
    this->offset    = 0;

    // this->cache is going to be used a lot.
    _prefetch(this->cache, 0, 1);
}

FileCache::~FileCache()
{
    delete[] this->cache;
}

std::string FileCache::readFile(std::string filename)
{
    auto it = fileMap.find(filename);
    if (_unlikely(it == fileMap.end()))
    {
        std::ifstream file(filename);
        if (!file)
            err("Failed to open " + filename);

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string data = buffer.str();

        std::this_thread::yield();

        if (data.size() > this->cacheSize)
            err("File too large for cache: " + filename);

        // Find free space
        std::memcpy(this->cache + this->offset, data.data(), data.size());

        FileDescriptor desc = {this->offset, (uint32_t)data.size()};
        fileMap[filename] = desc;

        this->offset += data.size();

        return data;
    }
    else
    {
        const FileDescriptor& desc = it->second;
        return std::string((char *)(this->cache + desc.offset), desc.size);
    }
}
