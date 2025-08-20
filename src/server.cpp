#include "server.hpp"

Log _log("log.log");

ThreadPool Server::pool{3};

Server::Server(std::string_view ip, const short port)
{
    Server::running = true;

    /* Hide cursor. Only in std::cout. */
    std::cout << "\033[?25l" << std::flush;

    Server::cache = std::make_shared<FileCache>(CACHE_SIZE);
    Server::http  = new HTTPResponseGenerator(std::move(Server::cache));

    /* Initialize OpenSSL. */
#ifdef USE_HTTPS
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    const SSL_METHOD *method = TLS_server_method();
    ssl_ctx = SSL_CTX_new(method);
    if (!ssl_ctx)
        err("Unable to create SSL context");

    /* Load certificate and private key. */
    if (_unlikely(SSL_CTX_use_certificate_file(ssl_ctx, CERT, SSL_FILETYPE_PEM) <= 0))
        err("Failed to load certificate");

    if (_unlikely(SSL_CTX_use_PrivateKey_file(ssl_ctx, KEY, SSL_FILETYPE_PEM) <= 0))
        err("Failed to load private key");
    
    if (_unlikely(!SSL_CTX_check_private_key(ssl_ctx)))
        err("Private key does not match the public certificate");

    _log << "SSL context initialized." << endline;
#endif

    /* Create a socket. */
    Server::servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_unlikely(Server::servfd == -1))
        err("Failed to create socket")
    else
        _log << "Successfully created socket" << endline;
    
    
    const int opt = 1;
    if (_unlikely(setsockopt(Server::servfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0))
        err("Failed to set socked options")
    else
        _log << "Successfully set socket options" << endline;
    
    /* Create the address. */
    Server::servaddr.sin_family      = AF_INET;
    Server::servaddr.sin_addr.s_addr = inet_addr(ip.data());
    Server::servaddr.sin_port        = htons(port);

    /* Ensure IP address is valid. */
    if (_unlikely(Server::servaddr.sin_addr.s_addr == INADDR_NONE))
        _log << "Invalid IP address" << endline;

    /* Bind the address to the socket. */
    if (_unlikely(bind(Server::servfd, (sockaddr *)&Server::servaddr, sizeof(Server::servaddr)) != 0))
        err("Failed to bind socket")
    else
        _log << "Successfully bound socket" << endline;

    /* Listen on the socket. */
    if (_unlikely(listen(Server::servfd, BACKLOG) != 0))
        err("Failed to listen on socket")
    else
        _log << "Server listening" << endline;
    
#ifdef USE_HTTPS
    _log << "Server UP at https://" << ip << ":" << port << "." << endline << endline;
#else
    _log << "Server UP at http://" << ip << ":" << port << "." << endline << endline;
#endif

    /* Flush the output. */
    _log << std::flush;
}

Server::~Server()
{
    _log << "Terminating Server instance" << endline;
    _log << "---------------------------" << endline;
    
#ifdef USE_HTTPS
    SSL_CTX_free(ssl_ctx);
    EVP_cleanup();
#endif

    /* Wait for all threads to terminate. */
    _log << "Waiting for all threads to terminate... ";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    _log << "Done." << endline;

    std::cout << "\033[?25h";

    /* Flush the output and show cursor. */
    _log << std::flush;
}

void Server::accept_clients()
{
    static socklen_t len = sizeof(sockaddr_in);

    std::thread
    (
        /* Main loop lambda function */
        []()
        {
            int clinum = 0;
            while (Server::running)
            {
                Client cli;

                /* Accept a new client connection. */
                cli.connfd = accept(Server::servfd, (sockaddr *)&cli.connaddr, (socklen_t *)&len);
                if (_unlikely(cli.connfd < 0))
                    err("Failed to accept client.")
                else
                {
                    if (_unlikely(++clinum % 1000 == 0))
                        _log << "Accepted client [#" << clinum / 1000 << "k]\r" << std::flush;
                }

#ifdef USE_HTTPS
                cli.ssl = SSL_new(ssl_ctx);
                if (_unlikely(!cli.ssl))
                {
                    ERR_print_errors_fp(stderr);
                    close(cli.connfd);
                    return;
                }

                if (_unlikely(SSL_set_fd(cli.ssl, cli.connfd) == 0))
                {
                    ERR_print_errors_fp(stderr);
                    SSL_free(cli.ssl);
                    close(cli.connfd);
                    return;
                }
#endif
                
                /* Spawn a new thread to handle the client. */
                Server::pool.enqueue(Server::handle_client, cli);
            }
        }
    ).detach();
}

void Server::handle_client(Client cli)
{
    thread_local char buff[READ_BUFF_SIZE];  // Avoid stack reallocation
    ssize_t size = 0;

#ifdef USE_HTTPS
    thread_local char peek[1];
    int n = recv(cli.connfd, peek, sizeof(peek), MSG_PEEK);

    if (n <= 0)
    {
        close(cli.connfd);
        return;
    }

    const bool https = (unsigned char)peek[0] == 0x16;

    if (https)
    {
        if (_unlikely(SSL_accept(cli.ssl) <= 0))
        {
            ERR_print_errors_fp(stderr);
            SSL_shutdown(cli.ssl);
            SSL_free(cli.ssl);
            close(cli.connfd);
            return;
        }

        size = SSL_read(cli.ssl, buff, READ_BUFF_SIZE);
    }
    else
        size = read(cli.connfd, buff, READ_BUFF_SIZE);

#else
    size = read(cli.connfd, buff, READ_BUFF_SIZE);
#endif

    if (_unlikely(size <= 0))
    {
        close(cli.connfd);
#ifdef USE_HTTPS
        SSL_shutdown(cli.ssl);
        SSL_free(cli.ssl);
#endif
        return;
    }

    // Use raw buffer instead of constructing string_view
    auto d = Server::http.load()->generate_response({buff, static_cast<size_t>(size)});

#ifdef USE_HTTPS
    if (https)
    {
        if (_unlikely(SSL_write(cli.ssl, d.first.c_str(), d.second) <= 0))
            err("Failed to send data via SSL");

        int shutdown_ret = SSL_shutdown(cli.ssl);
        if (shutdown_ret == 0)
            shutdown_ret = SSL_shutdown(cli.ssl);

        SSL_free(cli.ssl);
    }
    else
    {
        if (_unlikely(send(cli.connfd, d.first.c_str(), d.second, 0) <= 0))
            err("Failed to send data.");
    }
#else
    if (_unlikely(send(cli.connfd, d.first.c_str(), d.second, 0) <= 0))
        err("Failed to send data.");
#endif

    close(cli.connfd);
}
