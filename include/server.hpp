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

#ifdef USE_HTTPS
#include <openssl/ssl.h>
#include <openssl/err.h>

#endif

#include "utils/utils.hpp"
#include "utils/log.hpp"
#include "file.hpp"
#include "http.hpp"

#define CACHE_SIZE 4096
#define READ_BUFF_SIZE 8192

#define BACKLOG 128

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
    static sockaddr_in servaddr;

    static bool running;
    static std::atomic<HTTPResponseGenerator *> http;

    static std::shared_ptr<FileCache> cache;

#ifdef USE_HTTPS
    static SSL_CTX* ssl_ctx;
#endif

    /* Handle a client */
    _nnull(1)
    static inline
    void handle_client(Client* __restrict__ cli);

public:
    Server(std::string_view ip, const short port);
    ~Server();

    _throw
    static
    void accept_clients();
};

#endif