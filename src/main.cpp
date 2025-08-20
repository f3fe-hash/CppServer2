#include <iostream>
#include "server.hpp"

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        _log << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        _log << "Args: " << argv[0] << " " << argv[1] << " " << argv[2] << std::endl;
        return 1;
    }
    else
        _log << "Running: " << argv[0] << " " << argv[1] << " " << argv[2] << std::endl;

    Server server(argv[1], static_cast<short>(std::atoi(argv[2])));
    server.accept_clients();

    // Block main thread to keep server alive
    while (true) std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}