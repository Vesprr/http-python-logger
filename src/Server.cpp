#include "Server.hpp"

Server::Server(uint16_t port, uint16_t refreshIntervalMs)
    : m_port(port), m_refreshIntervalMs(refreshIntervalMs)
{
    crow::mustache::set_base("templates");

    // setup routes
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
        ctx["refreshInterval"] = m_refreshIntervalMs;

        auto page = crow::mustache::load("server.html");
        return page.render(ctx);
    });

    CROW_WEBSOCKET_ROUTE(appRef, "/ws")
    // handling connections in such a way that m_active_connections are always valid
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
        // no mutex needed here as no member global member is accessed
        conn.send_binary(encodeLogMessage(0xFFFF, 0, "", "", "RECIEVE", data));
        std::cout << "Receive message sent response: \n" << encodeLogMessage(0xFFFF, 0, "", "", "RECIEVE", data) << " to sender. \n";
    });
    // clang-format on
}

Server::~Server() { Stop(); }

void Server::m_tSendLogs()
{
    // keeping the `m_logSenderThread` alive
    while (m_running)
    {
        std::unique_lock<std::mutex> lock(m_logQueueMutex);

        // `wait` this blocks the thread until it is notified
        // when notified it evaluates the predicate
        // if true, the code after `wait` executes
        // otherwise the thread continues to sleep; analogous to `continue;`, code after is not executed
        // clang-format off
        m_logSenderNotifier.wait(lock, [this]{
            // only continue if there is something to log; then logs it
            // OR the server is stopping; exits gracefully
            return !m_logQueue.empty() || !m_running;
            // || !m_running is used instead of && m_running in the case that the server is starting to shut down
            // i.e at line 112 on main thread
            // the server has not stopped but m_running is false thus the thread will continue
            // and execute any logic that for gracefully stopping this thread
            // here it clears up the queue; messages will be sent to active clients
        });
        // clang-format on

        while (!m_logQueue.empty())
        {
            // gets the latest message from queue; and remove it from queue
            auto message = m_logQueue.front();
            m_logQueue.pop();

            // lock is no longer needed; thus it is unlocked
            // if not unlocked lock will be held even when the message is being sent
            // this will not allow other threads to use the queue and thus wasting time
            lock.unlock();

            // lock_guard is released when scope ends
            std::lock_guard<std::mutex> wsLock(m_ws_mutex);

            // making a snapshot for iterating
            auto conns = m_active_connections;
            // snapshot is taken as the method is async and m_active_connections may change while iterating through the list
            // which may lead dereferencing invalid memory

            // sending the messages
            // `conns` are valid here; only valid connections are held in the set
            for (auto *conn : conns)
            {
                // try catch is still used even for mutex because
                // mutex garentees our program doesn't access invalid memory
                // but if a connection is closed, and this mutex is acquired in our memory 
                // that connection is still valid and message will be sent anyways
                // wheather crows handles this is unsure
                // thus try catch is used
                try
                {
                    conn->send_binary(message);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Failed to send to a client: " << e.what() << "\n";
                }
            }

            // lock for the next loop
            lock.lock();
        }
    }
}

void Server::Start()
{
    m_running = true;

    // start logging queuer thread
    m_logSenderThread = std::thread(&Server::m_tSendLogs, this);

    // run crow app on `m_crowThread`
    // clang-format off
    m_crowThread = std::thread([this]() {
        appRef.port(m_port).multithreaded().run();
    });
    // clang-format on
}

void Server::Stop()
{
    m_running = false;

    m_logSenderNotifier.notify_all();

    // stop Crow
    appRef.stop();
    // join the crow thread to main thread; essentially stops the thread from running again
    if (m_crowThread.joinable())
        m_crowThread.join();

    // join the logSender thread
    if (m_logSenderThread.joinable())
        m_logSenderThread.join();
}

void Server::m_RegisterLogger(uint16_t id, const std::string &title)
{
    // store logger title
    m_loggers[id] = title;

    // no more processing is required
    std::cout << "Registered Logger: [" << std::to_string(id) << "] = " << title << "\n";
}

void Server::m_Log(const std::string &message)
{
    std::lock_guard<std::mutex> lock(m_logQueueMutex);

    // add the message to queue and notify
    m_logQueue.push(message);
    // wake only one thread as only one thread can send a message
    m_logSenderNotifier.notify_one();
    // `notify_one` notifies only one thread from all the threads waiting for the notification
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
    // next 8 bytes: time
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
    // 8 bytes to encode timestamp [year(2),month(1),day(1),hour(1),minute(1),milliseconds(2)]
    constexpr size_t TIME_BYTES = 8;
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
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm *tm_ptr = std::localtime(&now_time_t);
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
    // MILLISECONDS (not sending seconds but milliseconds only)
    auto ms_since_min = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch() % std::chrono::minutes(1));
    uint16_t ms = static_cast<uint16_t>(ms_since_min.count());
    buffer[14] = static_cast<char>(ms >> 8);
    buffer[15] = static_cast<char>(ms & 0xFF);
    // above static_cast<char> gets the least significant byte
    // and since the values from month and below can be stored in one byte
    // conversion to big-endian is unnecessary

    // - COPYING `logLevel` into the bufffer (LOG_LEVEL_LEN_BYTES)
    // prev bytes 15 bytes are already occupied
    memcpy(buffer.data() + 16,
           level.data(), level.size());
    // buffer filled from index 16 to 16 + level.size()
    // since memory is intialized to 0 on constructin buffer string
    // we do not need to worry about preinitializing value

    // - COPYING `func_name` (func_len)
    // prev (16 + LOG_LEVEL_LEN_BYTES) bytes are already occupied
    memcpy(buffer.data() + 16 + LOG_LEVEL_LEN_BYTES,
           func_name.c_str(), func_len);

    // - COPYING `file_name`
    // prev (16 + LOG_LEVEL_LEN_BYTES + func_len) bytes are already occupied
    memcpy(buffer.data() + 16 + LOG_LEVEL_LEN_BYTES + func_len,
           file_name.c_str(), file_len);

    // - COPYING `message` (file_len)
    // prev (16 + LOG_LEVEL_LEN_BYTES + func_len + file_len) bytes are already occupied
    memcpy(buffer.data() + 16 + LOG_LEVEL_LEN_BYTES + func_len + file_len,
           message.c_str(), message_len);

    return buffer;
}
