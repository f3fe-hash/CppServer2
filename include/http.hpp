#pragma once

#include <string>
#include <cstring>

#include <regex>
#include <unordered_map>

#include <atomic>

#include "utils/utils.hpp"
#include "io/file.hpp"

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
} HTTPRequest;

class HTTPRequestParser
{
    static inline const std::regex httpRequestRegex = std::regex(
        "^(GET|POST|PUT|DELETE|PATCH|OPTIONS|HEAD)\\s+(\\S+)\\s+HTTP/(\\d\\.\\d)\\r?\\n"
        "((?:[^\\r\\n]+:\\s*[^\\r\\n]*\\r?\\n)*)\\r?\\n([\\s\\S]*)",
        std::regex::ECMAScript | std::regex::icase | std::regex_constants::optimize);

    static inline const std::regex headerRegex = std::regex(
        "([^:\\r\\n]+):\\s*(.*)",
        std::regex::ECMAScript | std::regex_constants::optimize);
    
    static std::unordered_map<std::string, std::string> mimeTypes;
    
public:
    HTTPRequestParser() = default;
    ~HTTPRequestParser() = default;

    HTTPRequest parseRequest(std::string_view req);

    std::string getMimeType(const std::string& path);
};

class HTTPResponseGenerator
{
    static std::shared_ptr<FileCache> cache;

    static inline HTTPRequestParser parser;

    static std::unordered_map<int, std::string> HTTPCodes;

public:
    HTTPResponseGenerator(std::shared_ptr<FileCache> cache);
    ~HTTPResponseGenerator() = default;

    _throw _wur
    static inline
    HTTPResponse _generate_response(std::string_view data, std::string_view contentType, int errCode)
    {
        auto it = HTTPCodes.find(errCode);
        const std::string_view msg = (it != HTTPCodes.end()) ? it->second : "Unknown";

        /* Generate the string. */
        std::string res;
        res.reserve(data.size() + 128);
        res += "HTTP/1.1 " + std::to_string(errCode) + " " + std::string(msg) + "\r\n";
        res += "Content-Type: " + std::string(contentType) + "; charset=UTF-8\r\n";
        res += "Content-Length: " + std::to_string(data.size()) + "\r\n";
        res += "Connection: close\r\n\r\n";
        res += std::string(data);

        return {std::move(res), res.size()};
    }

    //
    // CHANGE THIS FUNCTION TO CHANGE SERVER FUNCTIONALITY
    //
    _wur
    static inline
    HTTPResponse generate_response(std::string_view request)
    {
        HTTPRequest req = HTTPResponseGenerator::parser.parseRequest(request);

        // Protect against ..
        if (req.path.find("..") != std::string::npos)
        {
            std::string err = cache->readFile("site/err/403.html");
            return HTTPResponseGenerator::_generate_response(err.empty() ? "<h1>403 Forbidden</h1>" : err, "text/html", 403);
        }
        
        // Ensure proper path
        if (req.path == "/" || req.path.empty())
            req.path = "index.html";
        else
            req.path = req.path[0] == '/' ? req.path.substr(1) : req.path;

        std::string data = cache->readFile("site/" + req.path);
        return HTTPResponseGenerator::_generate_response(data,
            HTTPResponseGenerator::parser.getMimeType(req.path), data.empty() ? 404 : 200);
    }
};
