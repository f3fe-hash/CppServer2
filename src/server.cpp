#include "server.hpp"

Log _log("log.log");

/* Define static variables. */
int                      Server::servfd   = 0;
sockaddr_in*             Server::servaddr = nullptr;

std::atomic<bool>                    Server::running(true);
std::atomic<HTTPResponseGenerator *> Server::http;

std::shared_ptr<FileCache> Server::cache;

Server::Server(std::string ip, short port)
{
    /* Hide cursor. Only in std::cout. */
    std::cout << "\033[?25l" << std::flush;

    Server::cache = std::make_shared<FileCache>(CACHE_SIZE);
    Server::http  = new HTTPResponseGenerator(Server::cache);

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
        _log << "Successfully binded socket" << endline;

    /* Listen on the socket. */
    if (_unlikely(listen(Server::servfd, 5) != 0))
        err("Failed to listen on socket")
    else
        _log << "Server listening" << endline;
    
    _log << "Server UP." << endline << endline;

    /* Flush the output. */
    std::cout << std::flush;
}

Server::~Server()
{
    _log << "Terminating Server instance" << endline;
    _log << "---------------------------" << endline;
    
    /* Wait for all threads to terminate. */
    _log << "Waiting for all threads to terminate... ";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    _log << "Done." << endline;

    /* Deallocate the server's address. */
    if (_likely(servaddr))
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
    char buff[READ_BUFF_SIZE];
    size_t size;
    if (_unlikely((size = read(cli->connfd, buff, READ_BUFF_SIZE)) <= 0))
        err("Failed to read data")
    
    auto d = Server::http.load()->generate_response(std::string(buff));

    /* Send the data. */
    if (_unlikely(send(cli->connfd, d.first.c_str(), d.second, 0) <= 0))
        err("Failed to send data.")

    /* Close connection and deallocate. */
    close(cli->connfd);
    delete cli;

    cli = nullptr;
}