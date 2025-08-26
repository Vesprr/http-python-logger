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
    Logger loggerC(server, 3, "LoggerC");
    Logger loggerD(server, 4, "LoggerD");

    server.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // Logging loop in main thread
    for (unsigned int i = 1; i <= 100; i++)
    {
        LOG_INFO(loggerA, "Message A " + std::to_string(i));
        LOG_WARN(loggerB, "Message B " + std::to_string(i));
        LOG_ERROR(loggerC, "Message C " + std::to_string(i));

        LOG_CUSTOM(loggerD, "Some Custom Message", "abcdefghghijkl");

        std::cout << "Logs sent; i = " << i << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cin.get();
}
