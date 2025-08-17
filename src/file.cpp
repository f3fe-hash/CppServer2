#include "file.hpp"

FileCache::FileCache(uint32_t cacheSize)
{
    this->cache     = new uint8_t[cacheSize];
    this->cacheSize = cacheSize;
    this->offset    = 0;
}

FileCache::~FileCache()
{
    delete[] this->cache;
}

std::pair<std::string, uint32_t> FileCache::readFile(std::string filename)
{
    _prefetch(this->cache, 0, 2);

    auto it = fileMap.find(filename);
    if (_unlikely(it == fileMap.end()))
    {
        std::ifstream file(filename);
        if (!file)
            _log << "Failed to open " << filename << std::endl;

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string data = buffer.str();

        std::this_thread::yield();

        uint32_t size = data.size();

        // Don't throw an error. Just don't cache
        if (size > this->cacheSize)
            return {data, size};

        // Find free space
        std::memcpy(this->cache + this->offset, data.data(), size);

        FileDescriptor desc = {this->offset, size};
        fileMap[filename] = desc;

        this->offset += size;

        return {data, size};
    }
    else
    {
        const FileDescriptor& desc = it->second;
        return {std::string((char *)(this->cache + desc.offset), desc.size), desc.size};
    }
}
