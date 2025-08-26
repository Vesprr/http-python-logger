#include "Server.hpp"

Server::Server(uint16_t port) : m_port(port) { m_Initialize(); }

Server::~Server() { Stop(); }

void Server::m_Initialize()
{
    crow::mustache::set_base("templates");

    // clang-format off
    CROW_ROUTE(appRef, "/")([this]() {
        crow::mustache::context ctx;

        // Build array directly inside ctx["m_loggers"]
        int idx = 0;
        for (const auto& [id, name] : m_loggers) {
            ctx["m_loggers"][idx]["id"] = id;
            ctx["m_loggers"][idx]["name"] = name;
            idx++;
        }

        ctx["port"] = m_port;

        auto page = crow::mustache::load("server.html");
        return page.render(ctx);
    });

    CROW_WEBSOCKET_ROUTE(appRef, "/ws")
    .onopen([&](crow::websocket::connection& conn){
        std::lock_guard<std::mutex> lock(m_ws_mutex);
        m_active_connections.insert(&conn);
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t){
        std::lock_guard<std::mutex> lock(m_ws_mutex);
        m_active_connections.erase(&conn);
    })
    // triggered when a message is recieved; not at sent
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary){
        conn.send_binary(getLogFormat(data, 0xFFFF, "RECIEVE", "", "", 0));
        std::cout << "Receive message sent response: \n" << getLogFormat(data, 0xFFFF, "RECIEVE", "", "", 0) << " to sender. \n";
    });
    // clang-format on
}

void Server::m_ProcessLogs()
{
    while (m_running)
    {
        std::unique_lock<std::mutex> lock(m_logQueueMutex);
        // clang-format off
        m_logCV.wait(lock, [this]{
             return !m_logQueue.empty() || !m_running;
        });
        // clang-format on

        while (!m_logQueue.empty())
        {
            auto message = m_logQueue.front();
            m_logQueue.pop();

            lock.unlock();

            // Send to all WebSocket clients
            std::lock_guard<std::mutex> wsLock(m_ws_mutex);

            // making a snapshot for iterating
            auto conns = m_active_connections;
            // snapshot is taking as the methods is async and m_active_connections may change while iterating through the list
            // which may lead dereferencing invalid memory

            for (auto *conn : conns)
            {
                conn->send_binary(message);
            }

            lock.lock();
        }
    }
}

void Server::Start()
{
    m_running = true;

    // Start logging thread
    m_logThread = std::thread(&Server::m_ProcessLogs, this);

    // clang-format off
    m_serverThread = std::thread([this]() {
        appRef.port(m_port).multithreaded().run();
    });
    // clang-format on
}

void Server::Stop()
{
    m_running = false;
    m_logCV.notify_all();

    // Stop Crow server
    appRef.stop(); // stops the server loop
    if (m_serverThread.joinable())
        m_serverThread.join();

    // Stop logging thread
    if (m_logThread.joinable())
        m_logThread.join();
}

void Server::RegisterLogger(uint16_t id, const std::string &title)
{
    // store logger title
    m_loggers[id] = title;

    std::cout << "Registered Logger: [" << std::to_string(id) << "] = " << title << "\n";
}

/// @brief Adds the message to a queue, from which messages are sent to. all client asynchronously
/// @param message The formatted message
void Server::m_Log(const std::string &message)
{
    std::lock_guard<std::mutex> lock(m_logQueueMutex);
    m_logQueue.push(message);
    m_logCV.notify_one();
}
