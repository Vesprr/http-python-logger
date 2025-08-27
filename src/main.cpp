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
    Logger loggerE(server, 5, "LoggerE");

    server.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // Logging loop in main thread
    for (unsigned int i = 1; i <= 100; i++)
    {
        LOG_INFO(loggerA, "Message from Logger A" + std::to_string(i));
        LOG_WARN(loggerB, "Message from Logger B" + std::to_string(i));
        LOG_ERROR(loggerC, "Message from Logger C" + std::to_string(i));

        LOG_CUSTOM(loggerD, "Some Custom Message that is mildly long", "abcdefghghijkl");
        LOG_CUSTOM(loggerD, "Another Custom Message from the same logger That is extremely long and may not fit into the width of the web client log terminal.", "abcdefghghqwewqasfdasdf");

        LOG_CUSTOM(loggerE, "A message with another custom level to test color, This one is only shorter than 8 characters.", "CUSTOM");

        std::cout << "Logs sent; i = " << i << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cin.get();
}
