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

    void Log(const std::string &message, uint16_t id);

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
    /// - First two bytes are ID (in big-endian format)
    ///
    /// - Rest of bytes are message bytes
    ///
    /// @param message The message to encode
    /// @param logger_id The id of the Logger to encode
    /// @return `std::string` containing the result string; Crow takes in std::string as input in `connection.send_binary(* MESSAGE_STR *)`
    inline static std::string getLogFormat(const std::string &message, uint16_t logger_id)
    {
        // crow.send_binary takes in std::string as well
        // also networking so not much of an issue
        // as bigger bottlenecks exists
        std::string buffer;
        // 2 bytes of id + N bytes of message
        buffer.resize(2 + message.size());

        // sending id as big-endian format; as is standard
        // some cpus use little-endian (0x1234 as [0x34, 0x12]) and other use big-endian (0x1234 as [0x12, 0x34])
        buffer[0] = static_cast<char>(logger_id >> 8);
        buffer[1] = static_cast<char>(logger_id & 0xFF);
        // working of above encoding:
        // suppose we need to send 0x1234
        // 1. shift right, 8 bits i.e >> 8
        // logger id becomes 0x0012; static_cast to char to truncate higher bits and get 0x12
        // store as first byte i.e buffer[0]
        // 2. mask 0x1234 with 0x00FF (same as 0xFF; hexadecimal puts bits from right)
        // we get 0x0034; static_cast to truncate higher bits and get 0x34
        // store as second byt i.e buffer[1]

        // copy the message into the buffer, after the two bytes
        std::copy(message.begin(), message.end(), buffer.begin() + 2);

        return buffer;
    }
};
