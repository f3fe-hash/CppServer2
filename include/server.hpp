#pragma once

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
#include "io/file.hpp"
#include "io/threads.hpp"
#include "http.hpp"

#define READ_BUFF_SIZE 8192

#define CACHE_SIZE  4096
#define NUM_THREADS 24
#define BACKLOG     128

/* Contains 1 client. */
using Client = struct
{
    int connfd;
    sockaddr_in connaddr;
#ifdef USE_HTTPS
    SSL* ssl;
#endif

    enum connType
    {
        CONNECTION_OPEN,
        CONNECTION_CLOSED
    };
};

class Server
{
    inline static int servfd;
    inline static sockaddr_in servaddr;

    inline static bool running;
    inline static std::atomic<HTTPResponseGenerator *> http;

    inline static std::shared_ptr<FileCache> cache;

    static ThreadPool pool;

#ifdef USE_HTTPS
    inline static SSL_CTX* ssl_ctx = nullptr;
#endif

    /* Handle a client */
    static inline
    void handle_client(Client cli);

public:
    Server(std::string_view ip, const short port);
    ~Server();

    _throw
    static
    void accept_clients();
};
