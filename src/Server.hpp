#pragma once
#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <crow.h>

/// @brief Responsible for creating the http server on localhost:`port` with title `title`.
class Server
{
private:
    uint16_t m_port;
    std::unordered_map<int, std::string> m_loggers;

    // CROW WEBSOCKETS
    // mutex for websockets
    std::mutex m_ws_mutex;
    // list of currently open connections
    std::unordered_set<crow::websocket::connection *> m_active_connections;

    std::thread m_logThread;
    std::queue<std::string> m_logQueue;
    std::mutex m_logQueueMutex;
    std::condition_variable m_logCV;
    std::atomic<bool> m_running;

    std::thread m_serverThread;

    crow::SimpleApp appRef;

    /// @brief Serve an http page at `localhost:port`, Setup websocket to recieve requests there.
    void m_Initialize();
    void m_ProcessLogs();

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
};
