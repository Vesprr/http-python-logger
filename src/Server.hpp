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
    /// - First two bytes are ID (in big-endian format)
    ///
    /// - Rest of bytes are formatted message bytes.
    /// Messages for formatted as [time] [level] [file:line func] message
    ///
    /// @param message The message to encode
    /// @param logger_id The id of the Logger to encode
    /// @param level String for Log Level
    /// @param file Name of file the `log` is called from
    /// @param func Name of function `log` is called from
    /// @param line Line Number the `log` is called from
    /// @return `std::string` containing the result string; Crow takes in std::string as input in `connection.send_binary(* MESSAGE_STR *)`
    inline static std::string encodeLogMessage(const std::string &message,
                                           uint16_t logger_id,
                                           const std::string &level,
                                           const std::string &file,
                                           const std::string &func,
                                           int line)
    {
        // timestamp
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        // build metadata string
        // "[time] [level] [file:line func] " trailing space to add padding when message is appended
        std::ostringstream meta;
        meta << "[" << std::put_time(std::localtime(&now_time_t), "%F %T") << "]"
             << " [" << level << "]"
             << " [" << file << ":" << line << " " << func << "] ";

        // crow.send_binary takes in std::string as well
        // also networking, so not much of an issue; as bigger bottlenecks exists
        std::string buffer;
        // 2 bytes of id + N1 bytes of meta + N2 bytes of message
        buffer.resize(2 + meta.str().size() + message.size());

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

        // copy the meta, after the two bytes of id
        // meta.str() returns reference to temporary string
        std::string meta_str = meta.str();
        memcpy(buffer.data() + 2, meta_str.data(), meta_str.size());
        // copy the message after the two bytes of id + string length of meta
        memcpy(buffer.data() + 2 + meta_str.size(), message.data(), message.size());
        // syntax: memcpy(dst*, src*, size)
        // copies data from `src` to `src + size` to `dst`
        // size = width of data to copy

        return buffer;
    }
};
