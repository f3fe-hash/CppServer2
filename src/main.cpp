#include <iostream>

#include "server.hpp"

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    Server* server = new Server("192.168.1.30", 8080);
    server->accept_clients();
    std::this_thread::sleep_for(std::chrono::seconds(100));

    delete server;
    
    return 0;
}