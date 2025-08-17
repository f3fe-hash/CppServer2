#ifndef __HTTP_HPP__
#define __HTTP_HPP__

#include <string>
#include <cstring>

#include <regex>
#include <unordered_map>

#include <atomic>

#include "utils.hpp"
#include "file.hpp"

using HTTPResponse = std::pair<std::string, size_t>;

typedef struct
{
    std::string request;
    std::string method;
    std::string path;
    std::string query;
    std::string http_version;

    std::unordered_map<std::string, std::string> headers;

    std::string payload;
    size_t payload_size;
} HTTPRequest;

class HTTPRequestParser
{
    std::regex httpRequestRegex;
    std::regex headerRegex;

public:
    HTTPRequestParser();
    ~HTTPRequestParser();

    HTTPRequest parseRequest(std::string req);
};

class HTTPResponseGenerator
{
    static constexpr const char* HTTP_HEADER_TEMPLATE =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Content-Length: %lu\r\n"
    "Connection: close\r\n"
    "\r\n%s";

    static size_t HTTP_HEADER_SIZE;

    static std::shared_ptr<FileCache> cache;

    static HTTPRequestParser* parser;

public:
    HTTPResponseGenerator(std::shared_ptr<FileCache> cache);
    ~HTTPResponseGenerator();

    _throw _wur
    static inline
    HTTPResponse _generate_response(std::string data)
    {
        /* Generate the string. */
        size_t total_size = data.size() + HTTPResponseGenerator::HTTP_HEADER_SIZE;
        char* res = new char[total_size];
        std::snprintf(res, total_size, HTTPResponseGenerator::HTTP_HEADER_TEMPLATE,
            data.size(), data.c_str());
        
        return {std::string(res), total_size};
    }

    //
    // CHANGE THIS FUNCTION TO CHANGE SERVER FUNCTIONALITY
    //
    _wur
    static inline
    HTTPResponse generate_response(std::string data)
    {
        HTTPRequest req = HTTPResponseGenerator::parser->parseRequest(data);

        // Protect against ..
        if (req.path.find("..") != std::string::npos)
            return HTTPResponseGenerator::_generate_response("403 Forbidden");
        
        // Ensure proper path
        std::string path;
        if (req.path == "/" || req.path.empty())
            path = "index.html";
        else
            path = req.path[0] == '/' ? req.path.substr(1) : req.path;
        std::string dat = cache->readFile("site/" + path);

        return HTTPResponseGenerator::_generate_response(dat);
    }
};

#endif