#include "server.hpp"

Log _log("log.log");

/* Define static variables. */
int                      Server::servfd   = 0;
sockaddr_in*             Server::servaddr = nullptr;

std::atomic<bool>                    Server::running(true);
std::atomic<HTTPResponseGenerator *> Server::http;

std::shared_ptr<FileCache> Server::cache;

#ifdef USE_HTTPS
SSL_CTX* Server::ssl_ctx = nullptr;
#endif

Server::Server(std::string ip, short port)
{
    /* Hide cursor. Only in std::cout. */
    std::cout << "\033[?25l" << std::flush;

    Server::cache = std::make_shared<FileCache>(CACHE_SIZE);
    Server::http  = new HTTPResponseGenerator(Server::cache);

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
    
    
    int opt = 1;
    if (_unlikely(setsockopt(Server::servfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0))
        err("Failed to set socked options")
    else
        _log << "Successfully set socket options" << endline;
    
    /* Create the address. */
    Server::servaddr = new sockaddr_in();
    Server::servaddr->sin_family      = AF_INET;
    Server::servaddr->sin_addr.s_addr = inet_addr(ip.c_str());
    Server::servaddr->sin_port        = htons(port);

    /* Ensure IP address is valid. */
    if (_unlikely(Server::servaddr->sin_addr.s_addr == INADDR_NONE))
        err("Invalid IP address");

    /* Bind the address to the socket. */
    if (_unlikely(bind(Server::servfd, (sockaddr *)Server::servaddr, sizeof(*Server::servaddr)) != 0))
        err("Failed to bind socket")
    else
        _log << "Successfully bound socket" << endline;

    /* Listen on the socket. */
    if (_unlikely(listen(Server::servfd, 5) != 0))
        err("Failed to listen on socket")
    else
        _log << "Server listening" << endline;
    
    
    
    _log << "Server UP at " << ip << ":" << port << "." << endline << endline;

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

    /* Deallocate the server's address. */
    delete servaddr;

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
            for (int clinum = 0; Server::running; clinum++)
            {
                Client* cli = new Client();

                /* Accept a new client connection. */
                cli->connfd = accept(Server::servfd, (sockaddr *)&cli->connaddr, (socklen_t *)&len);
                if (_unlikely(cli->connfd < 0))
                    err("Failed to accept client.")
                else
                {
                    if (_unlikely(clinum % 1000 == 0))
                        _log << "Accepted client [#" << clinum / 1000 << "k]\r" << std::flush;
                }
                
                /* Spawn a new thread to handle the client. */
                std::thread
                (
                    Server::handle_client,
                    cli
                ).detach();
            }
        }
    ).detach();
}

void Server::handle_client(Client* __restrict__ cli)
{
    char buff[READ_BUFF_SIZE] = {};
    size_t size = 0;

#ifdef USE_HTTPS
    SSL* ssl = SSL_new(ssl_ctx);
    if (_unlikely(!ssl))
    {
        ERR_print_errors_fp(stderr);
        close(cli->connfd);
        delete cli;
        return;
    }

    if (_unlikely(SSL_set_fd(ssl, cli->connfd) == 0))
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(cli->connfd);
        delete cli;
        return;
    }

    if (_unlikely(SSL_accept(ssl) <= 0))
    {
        ERR_print_errors_fp(stderr);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(cli->connfd);
        delete cli;
        return;
    }

    size = SSL_read(ssl, buff, READ_BUFF_SIZE);
#else
    size = read(cli->connfd, buff, READ_BUFF_SIZE);
#endif

    if (_unlikely(size <= 0))
    {
        err("Failed to read data");
        close(cli->connfd);
#ifdef USE_HTTPS
        SSL_shutdown(ssl);
        SSL_free(ssl);
#endif
        delete cli;
        return;
    }

    auto d = Server::http.load()->generate_response(std::string(buff, size));

#ifdef USE_HTTPS
    if (_unlikely(SSL_write(ssl, d.first.c_str(), d.second) <= 0))
        err("Failed to send data via SSL");

    SSL_shutdown(ssl);
    SSL_free(ssl);
#else
    if (_unlikely(send(cli->connfd, d.first.c_str(), d.second, 0) <= 0))
        err("Failed to send data.");
#endif

    close(cli->connfd);
    delete cli;
}
