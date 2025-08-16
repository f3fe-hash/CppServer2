#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include <string>
#include <cstring>

#include <thread>
#include <atomic>
#include <functional>

#include <chrono>

#include "types.h"
#include "utils.hpp"
#include "file.hpp"

/* Contains 1 client. */
using Client = struct
{
    int connfd;
    sockaddr_in connaddr;

    enum connType
    {
        CONNECTION_OPEN,
        CONNECTION_CLOSED
    };
};

class Server
{
    static int servfd;
    static sockaddr_in* servaddr;

    static std::atomic<bool> running;

    static std::atomic<FileCache *> cache;

    /* Handle a client */
    _throw _nnull(1)
    static inline
    void handle_client(Client* __restrict__ cli);

public:
    Server(std::string ip, short port);
    ~Server();

    _throw
    static
    void accept_clients();
};

#endif