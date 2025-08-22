#include "http.hpp"

std::shared_ptr<FileCache> HTTPResponseGenerator::cache;

std::unordered_map<int, std::string> HTTPResponseGenerator::HTTPCodes =
{
    {200, "OK"},
    {403, "Forbidden"}
};

std::unordered_map<std::string, std::string> HTTPRequestParser::mimeTypes =
{
    // Applications
    {".js",   "application/javascript"},
    {".json", "application/json"},
    {".wasm", "application/wasm"},
    
    // Documents
    {".doc",  "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".pdf",  "application/pdf"},
    {".ppt",  "application/vnd.ms-powerpoint"},

    {".gz",  "application/gzip"},
    {".rar",  "application/vnd.rar"},
    {".zip", "application/zip"},

    // Audio
    {".midi", "audio/midi"},
    {".mid",  "audio/midi"},
    {".mp3",  "audio/mp3"},
    {".wav",  "audio/wav"},

    // Images
    {".gif",  "image/gif"},
    {".ico",  "image/x-icon"},
    {".jpg",  "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png",  "image/png"},
    {".svg",  "image/svg+xml"},
    {".webm", "image/webm"},

    // Text
    {".css",  "text/css"},
    {".csv",  "text/csv"},
    {".html", "text/html"},
    {".htm",  "text/html"},
    {".txt",  "text/plain"},

    // Video
    {".mp4", "video/mp4"},
    {".avi", "video/x-msvideo"},
    {".mov", "video/quicktime"},
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

        const std::string& headers_str = matches[4];

        for (std::sregex_iterator it(headers_str.begin(), headers_str.end(), headerRegex), end;
             it != end; ++it)
        {
            preq.headers.emplace((*it)[1].str(), (*it)[2].str());
        }
    }

    return preq;
}

std::string HTTPRequestParser::getMimeType(const std::string& path)
{

    // Find the last dot in the path
    std::size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        std::string ext = path.substr(dotPos);
        auto it = mimeTypes.find(ext);
        if (it != mimeTypes.end())
            return it->second;
    }

    return "application/octet-stream"; // default fallback
}
