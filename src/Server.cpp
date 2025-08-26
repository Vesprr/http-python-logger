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
        conn.send_binary(encodeLogMessage(0xFFFF, 0, "", "", "RECIEVE", data));
        std::cout << "Receive message sent response: \n" << encodeLogMessage(0xFFFF, 0, "", "", "RECIEVE", data) << " to sender. \n";
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

std::string Server::encodeLogMessage(uint16_t logger_id,
                                     int line_number,
                                     const std::string &func_name,
                                     const std::string &file_name,
                                     const std::string &level,
                                     const std::string &message)
{
    // first 2 bytes: logger id
    // next 2 bytes: line_number
    // next 2 bytes: length of func_name
    // next 2 bytes: length of file_name
    // next 8 bytes: level
    // next N bytes: time
    // next func_name bytes
    // next file_name bytes
    // next message bytes

    // using constexpr below; it is compile-time constant and will be optimized away by compiler
    // 2 bytes to encode `logger_id`
    constexpr size_t LOGGER_ID_BYTES = 2;
    // 2 bytes for encode `line_number`
    constexpr size_t LINE_NUMBER_BYTES = 2;
    // 2 bytes to encode the "length" of `function_name`
    constexpr size_t FUNC_NAME_LEN_BYTES = 2;
    // 2 bytes to encode the "length" of `file_name`
    constexpr size_t FILE_NAME_LEN_BYTES = 2;
    // 7 bytes to encode timestamp (year, month, day, hour, min, sec)
    constexpr size_t TIME_BYTES = 7;
    // 8 bytes/characters to encode `log_level`
    constexpr size_t LOG_LEVEL_LEN_BYTES = 8;

    // length of `func_name` bytes
    const uint16_t func_len = static_cast<uint16_t>(func_name.size());
    // length of `file_name` bytes
    const uint16_t file_len = static_cast<uint16_t>(file_name.size());
    // length of `message` bytes
    const uint16_t message_len = static_cast<uint16_t>(message.size());

    // total_size
    const size_t total_size = LOGGER_ID_BYTES + LINE_NUMBER_BYTES + FUNC_NAME_LEN_BYTES + FILE_NAME_LEN_BYTES + TIME_BYTES + LOG_LEVEL_LEN_BYTES + func_len + file_len + message.size();

    // ---

    // buffer to return as result
    std::string buffer(total_size, 0);

    // - ENCODING `logger_id` 2
    // using big-endian format; as is standard
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

    // - ENCODING `line_number` 2
    // using big-endian format
    buffer[2] = static_cast<char>(line_number >> 8);
    buffer[3] = static_cast<char>(line_number & 0xFF);

    // - ENCODING `func_len` 2
    // using big-endian format
    buffer[4] = static_cast<char>(func_len >> 8);
    buffer[5] = static_cast<char>(func_len & 0xFF);

    // - ENCODING `file_len` 2
    // using big-endian format
    buffer[6] = static_cast<char>(file_len >> 8);
    buffer[7] = static_cast<char>(file_len & 0xFF);

    // - ENCODING `time` 7
    // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::tm t = {};
    t.tm_year = 2025 - 1900; // tm_year = years since 1900
    t.tm_mon = 8 - 1;        // tm_mon = 0-based month
    t.tm_mday = 26;          // day of the month
    t.tm_hour = 23;
    t.tm_min = 30;
    t.tm_sec = 5;

    // 2. Convert tm to time_t
    std::time_t tt = std::mktime(&t); // interprets t as local time
    // std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm *tm_ptr = std::localtime(&tt);
    // YEAR
    // "+ 1900" since tm_year gives years since 1900 i.e for 2020 returns 120
    uint16_t year = static_cast<uint16_t>(tm_ptr->tm_year + 1900);
    buffer[8] = static_cast<char>(year >> 8);
    buffer[9] = static_cast<char>(year & 0xFF);
    // MONTH
    buffer[10] = static_cast<char>(tm_ptr->tm_mon + 1); // "+ 1" since tm_mon returns [0 - 11]
    // DAY
    buffer[11] = static_cast<char>(tm_ptr->tm_mday); // tm_mday returns [1 - 31]
    // HOUR
    buffer[12] = static_cast<char>(tm_ptr->tm_hour); // tm_hour returns [0 - 23]
    // MINUTE
    buffer[13] = static_cast<char>(tm_ptr->tm_min); // tm_min returns [0 - 59]
    // SECONDS
    buffer[14] = static_cast<char>(tm_ptr->tm_sec); // tm_sec returns [0 - 59]
    // above static_cast<char> gets the least significant byte
    // and since the values from month and below can be stored in one byte
    // conversion to big-endian is unnecessary

    // - COPYING `logLevel` into the bufffer (LOG_LEVEL_LEN_BYTES)
    // prev bytes 14 bytes are already occupied
    memcpy(buffer.data() + 15,
           level.data(), level.size());
    // buffer filled from index 15 to 15 + level.size()
    // since memory is intialized to 0 on constructin buffer string
    // we do not need to worry about preinitializing value

    // - COPYING `func_name` (func_len)
    // prev (14 + LOG_LEVEL_LEN_BYTES) bytes are already occupied
    memcpy(buffer.data() + 15 + LOG_LEVEL_LEN_BYTES,
           func_name.c_str(), func_len);

    // - COPYING `file_name`
    // prev (14 + LOG_LEVEL_LEN_BYTES + func_len) bytes are already occupied
    memcpy(buffer.data() + 15 + LOG_LEVEL_LEN_BYTES + func_len,
           file_name.c_str(), file_len);

    // - COPYING `message` (file_len)
    // prev (14 + LOG_LEVEL_LEN_BYTES + func_len + file_len) bytes are already occupied
    memcpy(buffer.data() + 15 + LOG_LEVEL_LEN_BYTES + func_len + file_len,
           message.c_str(), message_len);

    return buffer;
}
