#include <iostream>
#include <thread>
#include <chrono>
#include "Server.hpp"
#include "Logger.hpp"

int main()
{
    Server server(5000);
    Logger loggerA(server, 1, "LoggerA");
    Logger loggerB(server, 2, "LoggerB");

    server.Start();

    // for (unsigned int i = 0; true; i++)
    // {
    //     loggerA.log("Message A " + std::to_string(i));
    //     loggerB.log("Message B " + std::to_string(i));

    //     std::cout << "Logs sent; i = " << i << "\n";
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }
}
