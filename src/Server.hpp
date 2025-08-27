#pragma once
#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <crow.h>

#include "Logger.hpp"

// forward declare
class Logger;

/// @brief Responsible for creating the http server on localhost:`port` with title `title`.
class Server
{
private:
    uint16_t m_port;
    std::unordered_map<uint16_t, std::string> m_loggers;

    // CROW WEBSOCKETS
    // mutex for websockets
    std::mutex m_ws_mutex;
    // connections are owned by CROW and only referenced here
    std::unordered_set<crow::websocket::connection *> m_active_connections;

    std::thread m_logThread;
    std::queue<std::string> m_logQueue;
    std::mutex m_logQueueMutex;
    std::condition_variable m_logCV;
    std::atomic<bool> m_running{false};

    std::thread m_serverThread;

    crow::SimpleApp appRef;

    // give Logger access to private members
    friend class Logger;

    /// @brief Serve an http page at `localhost:port`, Setup websocket to recieve requests there.
    void m_Initialize();

    void m_ProcessLogs();

    void m_Log(const std::string &message);

public:
    Server() = delete;
    Server(uint16_t port);

    ~Server();

    void RegisterLogger(uint16_t id, const std::string &title);

    /// @brief Establish an Websocket Connection to `localhost:port`.
    void Start();
    /// @brief Closes Websocket connection.
    void Stop();

    /// @brief Encodes the message and given id to something that can be sent and decoded by the compilers.
    ///
    /// ### Encoded format:
    ///
    /// - First 2 bytes are "ID" (in big-endian format)
    ///
    /// - Next 2 bytes are "Line Number" (in big-endian format)
    ///
    /// - Next 2 bytes are "Length of "Function Name" String" (in big-endian format)
    ///
    /// - Next 2 bytes are "Length of File Name String" (in big-endian format)
    ///
    /// - Next 8 bytes are "Time" [year(2),month(1),day(1),hour(1),minute(1),milliseconds(2)]
    ///
    /// - Next 8 bytes are "Log Level"
    ///
    /// - Next N1 bytes are "Function Name"
    ///
    /// - Next N2 bytes are "File Name"
    ///
    /// - Next N3 bytes are "Message"
    ///
    /// @param logger_id The id of the Logger to encode
    /// @param line_number Line Number the `log` is called from
    /// @param func_name Name of function `log` is called from
    /// @param file_name Name of file_name the `log` is called from
    /// @param level String for Log Level
    /// @param message The message to encode
    /// @return `std::string` containing the result string; Crow takes in std::string as input in `connection.send_binary(* MESSAGE_STR *)`
    static std::string encodeLogMessage(uint16_t logger_id,
                                        int line_number,
                                        const std::string &func_name,
                                        const std::string &file_name,
                                        const std::string &level,
                                        const std::string &message);
};
