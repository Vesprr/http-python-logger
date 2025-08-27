#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include "Server.hpp"
#include "Logger.hpp"

// SAMPLE USAGE IN CPP
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

    auto start = std::chrono::high_resolution_clock::now();


    // sends 10k logs total
    // takes at least 11ms to send 10 logs
    // (1000/11)*10 requests per second ~ 1k requests per second
    for (unsigned int i = 1; i <= 1000; i++)
    {
        LOG_INFO(loggerA, "(BA) Message from Logger A " + std::to_string(i));
        LOG_WARN(loggerB, "(BA) Message from Logger B " + std::to_string(i));
        LOG_ERROR(loggerC, "(BA) Message from Logger C " + std::to_string(i));
        LOG_CUSTOM(loggerD, "(BA) Message from Logger D " + std::to_string(i), "CUSTOMA");
        LOG_CUSTOM(loggerE, "(BA) Message from Logger E " + std::to_string(i), "CUSTOMB");

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // added to check accuracy

        LOG_INFO(loggerA, "(BB) Message from Logger A " + std::to_string(i));
        LOG_WARN(loggerB, "(BB) Message from Logger B " + std::to_string(i));
        LOG_ERROR(loggerC, "(BB) Message from Logger C " + std::to_string(i));
        LOG_CUSTOM(loggerD, "(BB) Message from Logger D " + std::to_string(i), "CUSTOM1");
        LOG_CUSTOM(loggerE, "(BB) Message from Logger E " + std::to_string(i), "CUSTOM2");
        

        std::cout << "Logs sent; i = " << i << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Logging took " << static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) << "ms \n";

    std::cin.get();
}
