#include "server.hpp"

/* Define static variables. */
int                      Server::servfd   = 0;
sockaddr_in*             Server::servaddr = nullptr;
std::atomic<bool>        Server::running(true);
std::atomic<FileCache *> Server::cache;

Server::Server(std::string ip, short port)
{
    Server::cache = new FileCache(4096);

    /* Create a socket. */
    Server::servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (Server::servfd == -1)
        err("Failed to create socket")
    else
        std::cout << "Successfully created socket" << endline;
    
    /* Create the address. */
    Server::servaddr = new sockaddr_in();
    Server::servaddr->sin_family      = AF_INET;
    Server::servaddr->sin_addr.s_addr = inet_addr(ip.c_str());
    Server::servaddr->sin_port        = htons(port);

    /* Ensure IP address is valid. */
    if (Server::servaddr->sin_addr.s_addr == INADDR_NONE)
        err("Invalid IP address");

    /* Bind the address to the socket. */
    if ((bind(Server::servfd, (sockaddr *)Server::servaddr, sizeof(*Server::servaddr))) != 0)
        err("Failed to bind socket")
    else
        std::cout << "Successfully binded socket" << endline;

    /* Listen on the socket. */
    if ((listen(Server::servfd, 5)) != 0)
        err("Failed to listen on socket")
    else
        std::cout << "Server listening" << endline;
    
    std::cout << "Server UP." << endline << endline;

    /* Flush the output. */
    std::cout << std::flush;
}

Server::~Server()
{
    std::cout << "Terminating Server instance" << endline;
    std::cout << "---------------------------" << endline;
    
    /* Wait for all threads to terminate. */
    std::cout << "Waiting for all threads to terminate... ";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Done." << endline;

    /* Deallocate the server's address. */
    if (servaddr)
        delete servaddr;

    /* Flush the output. */
    std::cout << std::flush;
}

void Server::accept_clients()
{
    static socklen_t len = sizeof(sockaddr_in);

    std::thread
    (
        /* Main loop lambda function */
        []()
        {
            while (Server::running)
            {
                Client* cli = new Client();

                /* Accept a new client connection. */
                cli->connfd = accept(Server::servfd, (sockaddr *)&cli->connaddr, (socklen_t *)&len);
                if (_unlikely(cli->connfd < 0))
                    err("Failed to accept clients")
                else
                    std::cout << "Accepted client\r";
                
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
    std::string data = Server::cache.load()->readFile("site/index.html");

    /* Send the data. */
    unsigned long len = data.size();
    if (_unlikely(send(cli->connfd, data.c_str(), len, 0) <= 0))
        err("Failed to send data.")

    /* Close connection and deallocate. */
    close(cli->connfd);
    delete cli;

    cli = nullptr;
}