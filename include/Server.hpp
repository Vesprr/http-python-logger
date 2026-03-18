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
    // The port on which the server will open to serve the logs
    uint16_t m_port;
    /// How often the messages be refreshed on the CLIENT; only used to send a reference to CLIENT informing of user desired refresh interval
    ///
    /// Give value in `milliseconds`
    ///
    /// #### CONSIDERATIONS:
    /// - Keep in mind that this doesn't affect the sending process
    /// - It also doesn't affect the recieving process
    /// - Messages are sent and recieved as fast as possible
    /// - However to prevent DOM load on CLIENT side, a refresh interval on the orders of 100 ms is used to update the DOM; CLIENT is HTML page
    /// - See `assets/templates/server.html`
    uint16_t m_refreshIntervalMs;
    std::unordered_map<uint16_t, std::string> m_loggers;

    // CROW WEBSOCKETS
    // mutex for websockets
    // mutex doesn't allow multiple threads to access the same code block until one thread has finished working on it
    // used to prevent thread race conditions
    std::mutex m_ws_mutex;
    // connections are owned by CROW and only referenced here
    std::unordered_set<crow::websocket::connection *> m_active_connections;

    // THREAD: used to send the logs to the CLIENT
    std::thread m_logSenderThread;
    // used to send a notification that a message or some messages are ready to be sent
    std::condition_variable m_logSenderNotifier;
    std::atomic<bool> m_running{false};
    // atomic = light-weight version of mutex for single variable
    // multiple threads are reading or writing; so needs to be thread-safe

    /// @brief Runs of a seperate thread; sends the messages stored in the queue to the websockets recievers
    void m_tSendLogs();

    // log queue; when f_Log is called all messages are first queued first and sent using a seperate thread: `m_logSenderThread`
    std::queue<std::string> m_logQueue;
    std::mutex m_logQueueMutex; // see `m_ws_mutex`; rest self-explanatory

    // THREAD: used to run the crow server
    std::thread m_crowThread;
    // referece to crow app
    crow::SimpleApp appRef;

    // give Logger access to private members
    friend class Logger;

    /// @brief Adds the message to the queue; Uses `m_logSenderNotifier` to notify `m_logSenderThread`
    /// @param message The formatted message in binary; std::string is used as CROW takes that as argument for `conn->send_binary(std::string)`;
    void f_Log(const std::string &message);
    /// @brief Register the logger with given id and title
    /// @param id the id of logger uint16_t
    /// @param title the title of logger as std::string
    void f_RegisterLogger(uint16_t id, const std::string &title);

public:
    /// @brief Default contructor has been deleted: Use `Server(uint16_t port, uint16_t refreshIntervalMs = 50)` instead
    Server() = delete;
    /// @brief Initalizes `CROW` server by setting up routes at:
    ///
    /// - `http://localhost:"port"/`: For serving the logs
    ///
    /// - `ws://localhost:"port"/ws`: Setup websocket server for CLIENT to use
    ///
    /// @param port
    /// @param refreshIntervalMs
    Server(uint16_t port, uint16_t refreshIntervalMs = 50);

    ~Server();

    /// @brief Start the server.
    void Start();
    /// @brief Close the server.
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
