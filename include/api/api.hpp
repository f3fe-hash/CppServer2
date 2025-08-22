#pragma once

#include "api/api_base.hpp"

class API : BaseAPI
{
    static inline std::shared_ptr<FileCache> cache;

    static inline HTTPRequestParser httpParser;
    static HTTPResponseGenerator httpGen;
public:
    API(const std::string& ip, const short port) : BaseAPI(ip, port)
    {
        this->cache = s.getCache();
        this->httpGen = s.getHTTP();
    };

    Server s;

    HTTPResponse generateResponse(const HTTPRequest& req);
};