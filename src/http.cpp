#include "http.hpp"

std::shared_ptr<FileCache> HTTPResponseGenerator::cache;

std::unordered_map<int, std::string> HTTPResponseGenerator::HTTPCodes =
{
    {200, "OK"},
    {403, "Forbidden"}
};

HTTPResponseGenerator::HTTPResponseGenerator(std::shared_ptr<FileCache> cache)
{
    HTTPResponseGenerator::cache = cache;
}

HTTPRequest HTTPRequestParser::parseRequest(std::string_view req)
{
    HTTPRequest preq;
    preq.request = req;
    preq.headers.reserve(8);

    std::string req_copy(req);  // Needed for regex matching (null-terminated)
    std::smatch matches;

    if (_likely(std::regex_match(req_copy, matches, httpRequestRegex)))
    {
        preq.method       = matches[1].str();
        preq.path         = matches[2].str();
        preq.http_version = matches[3].str();

        const std::string& headers_str = matches[4]; // ✅ Must be std::string

        for (std::sregex_iterator it(headers_str.begin(), headers_str.end(), headerRegex), end;
             it != end; ++it)
        {
            preq.headers.emplace((*it)[1].str(), (*it)[2].str());
        }
    }

    return preq;
}
