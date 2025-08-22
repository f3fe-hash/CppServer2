#pragma once

#include <string>
#include <cstring>

#include <regex>
#include <unordered_map>

#include <atomic>
#include <functional>

#include "utils/utils.hpp"
#include "io/file.hpp"

using HTTPResponse = std::string;

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

using HTTPCallback = std::function<HTTPResponse(const HTTPRequest&)>;

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
    inline static HTTPRequestParser parser;

    static std::unordered_map<int, std::string> HTTPCodes;

    inline static HTTPCallback callback;

public:
    HTTPResponseGenerator(HTTPCallback callback);
    ~HTTPResponseGenerator() = default;

    _throw _wur _const
    static inline
    HTTPResponse _generate_response(std::string_view data, std::string_view contentType, int errCode)
    {
        auto it = HTTPCodes.find(errCode);
        const std::string& msg = (it != HTTPCodes.end()) ? it->second : "Unknown";

        std::string res;
        res.reserve(data.size() + 128);
        res += "HTTP/1.1 " + std::to_string(errCode) + " " + std::string(msg) + "\r\n";
        res += "Content-Type: " + std::string(contentType) + "; charset=UTF-8\r\n";
        res += "Content-Length: " + std::to_string(data.size()) + "\r\n";
        res += "Connection: close\r\n\r\n";
        res += std::string(data);

        return HTTPResponse(std::move(res));
    }

    //
    // CHANGE THIS FUNCTION TO CHANGE SERVER FUNCTIONALITY
    //
    _wur
    static inline
    HTTPResponse generate_response(std::string_view request)
    {
        const HTTPRequest& req = HTTPResponseGenerator::parser.parseRequest(request);
        return
            HTTPResponseGenerator::callback(req);
    }
};
