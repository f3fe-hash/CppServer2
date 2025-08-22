#include "api/api.hpp"

HTTPResponseGenerator API::httpGen{nullptr};

HTTPResponse API::generateResponse(const HTTPRequest& req)
{
    // Protect against ..
    if (_unlikely(req.path.find("..") != std::string::npos))
    {
        std::string err = cache->readFile("site/err/403.html");
        return API::httpGen._generate_response(err.empty() ? "<h1>403 Forbidden</h1>" : err, "text/html", 403);
    }
    
    // Ensure proper path
    std::string path = req.path;
    if (req.path == "/" || req.path.empty())
        path = "index.html";
    else
        path = req.path[0] == '/' ? req.path.substr(1) : req.path;

    std::string data = cache->readFile("site/" + path);
    if (_unlikely(data.empty()))
    {
        std::string err = cache->readFile("site/err/404.html");
        return API::httpGen._generate_response(err.empty() ? "<h1>404 Not Found</h1>" : err, "text/html", 404);
    }
    
    return API::httpGen._generate_response(data,
        API::httpParser.getMimeType(path), 200);
}