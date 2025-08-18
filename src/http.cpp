#include "http.hpp"

size_t HTTPResponseGenerator::HTTP_HEADER_SIZE;
std::shared_ptr<FileCache> HTTPResponseGenerator::cache;
HTTPRequestParser* HTTPResponseGenerator::parser;

std::unordered_map<int, std::string> HTTPResponseGenerator::HTTPCodes =
{
    {200, "OK"},
    {403, "Forbidden"}
};

HTTPResponseGenerator::HTTPResponseGenerator(std::shared_ptr<FileCache> cache)
{
    this->HTTP_HEADER_SIZE = std::strlen(this->HTTP_HEADER_TEMPLATE);
    this->cache = cache;

    /* Initialize the parser. */
    this->parser = new HTTPRequestParser();
}

HTTPResponseGenerator::~HTTPResponseGenerator()
{}

HTTPRequestParser::HTTPRequestParser()
{
    this->httpRequestRegex = std::regex(
    R"(^(GET|POST|PUT|DELETE|PATCH|OPTIONS|HEAD)\s+(\S+)\s+HTTP/(\d\.\d)\r?\n((?:[^\r\n]+:\s*[^\r\n]*\r?\n)*)\r?\n(.*)?)",
    std::regex::ECMAScript | std::regex::icase);

    this->headerRegex = std::regex(
    R"(([^:\r\n]+):\s*([^\r\n]+))"
    );
}

HTTPRequestParser::~HTTPRequestParser()
{}

HTTPRequest HTTPRequestParser::parseRequest(std::string req)
{
    HTTPRequest preq;
    preq.request = req;

    std::smatch matches;
    if (std::regex_match(req, matches, this->httpRequestRegex))
    {
        /* Query is in matches[2] as well as path. */
        preq.method       = matches[1];
        preq.path         = matches[2];
        preq.http_version = matches[3];

        std::string headers_str(matches[4]);

        /* Read the headers. */
        for (std::sregex_iterator it(headers_str.begin(), headers_str.end(), this->headerRegex), end; it != end; ++it)
            preq.headers.insert({(*it)[1], (*it)[2]});
    }

    return preq;
}
