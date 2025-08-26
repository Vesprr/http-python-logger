#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include "Server.hpp"
#include "Logger.hpp"

int main()
{
    Server server(5000);

    Logger loggerA(server, 1, "LoggerA");
    Logger loggerB(server, 2, "LoggerB");

    server.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // Logging loop in main thread
    for (unsigned int i = 1; i <= 100; i++)
    {
        loggerA.log("Message A " + std::to_string(i));
        loggerB.log("Message B " + std::to_string(i));

        std::cout << "Logs sent; i = " << i << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cin.get();
}
