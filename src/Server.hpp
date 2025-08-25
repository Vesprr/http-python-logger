#pragma once
#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <crow.h>

/// @brief Responsible for creating the http server on localhost:`port` with title `title`.
class Server
{
private:
    uint16_t m_port;
    std::unordered_map<int, std::string> m_loggers;

    crow::SimpleApp appRef;

    /// @brief Serve an http page at `localhost:port`, Setup websocket to recieve requests there.
    void m_Initialize();

public:
    Server() = delete;
    Server(uint16_t port);

    ~Server();

    void RegisterLogger(int id, const std::string &title);
    void Log(const std::string &message, int id);

    /// @brief Establish an Websocket Connection to `localhost:port`.
    void Start();
    /// @brief Closes Websocket connection.
    void Stop();

private: // PRIVATE HELPERS FUNCTIONS
    void m_SendMessage(const std::string &message, int id);
};
