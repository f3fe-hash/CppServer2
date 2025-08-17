#include <iostream>

#include "server.hpp"

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    Server* server = new Server("192.168.1.39", 8080);
    server->accept_clients();
    while (true);

    delete server;
    
    return 0;
}