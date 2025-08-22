#pragma once

#include "server/server.hpp"

class BaseAPI
{
public:
    BaseAPI(const std::string& ip, const short port) :
    s(ip, port, (HTTPCallback)[this](const HTTPRequest& req)
    {return generateResponse(req);}) {};

    Server s;

    virtual HTTPResponse generateResponse(const HTTPRequest& req) = 0;
};