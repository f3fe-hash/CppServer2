#include <iostream>
#include "server.hpp"

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
        return 1;
    }

    Server server(argv[1], static_cast<short>(std::atoi(argv[2])));
    server.accept_clients();

    // Block main thread to keep server alive
    while (true) std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}