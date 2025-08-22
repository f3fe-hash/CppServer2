#include <iostream>
#include "api/api.hpp"
#include "server/server.hpp"

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

    API api(argv[1], static_cast<short>(std::atoi(argv[2])));
    Server serv;
    api.s = serv;
    serv.accept_clients();

    // Block main thread to keep server alive
    while (true) std::this_thread::sleep_for(std::chrono::seconds(1000));

    return 0;
}