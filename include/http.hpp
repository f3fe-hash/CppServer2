#ifndef __HTTP_HPP__
#define __HTTP_HPP__

#include <string>
#include <cstring>

#include <regex>
#include <unordered_map>

#include <atomic>

#include "utils/utils.hpp"
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
        res.reserve(data.size() + 128); // Optional, for performance
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
    HTTPResponse generate_response(std::string_view data)
    {
        HTTPRequest req = HTTPResponseGenerator::parser.parseRequest(data);

        // Protect against ..
        if (req.path.find("..") != std::string::npos)
            return HTTPResponseGenerator::_generate_response("<h1>403 Forbidden</h1>", "text/html", 403);
        
        // Ensure proper path
        std::string path;
        if (req.path == "/" || req.path.empty())
            path = "index.html";
        else
            path = req.path[0] == '/' ? req.path.substr(1) : req.path;

        return HTTPResponseGenerator::_generate_response(cache->readFile("site/" + path),
            HTTPResponseGenerator::parser.getMimeType(path), 200);
    }
};

#endif