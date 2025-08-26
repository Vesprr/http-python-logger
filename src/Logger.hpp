#pragma once
#include <string>
#include <cstdint>

#include "Server.hpp"

class Server;

class Logger
{
private:
    Server &m_server;
    uint16_t m_logger_id;

public:
    Logger() = delete;
    Logger(Server &server, const std::string &title);
    Logger(Server &server, uint16_t logger_id, const std::string &title);

    void Log(const std::string &message,
             const std::string &level,
             const std::string &file_path,
             const std::string &func,
             int line);
};

// Macro wrapper: captures file/function/line automatically
#define LOG_INFO(logger, msg) logger.Log(msg, "INFO", __FILE__, __func__, __LINE__)
#define LOG_WARN(logger, msg) logger.Log(msg, "WARN", __FILE__, __func__, __LINE__)
#define LOG_ERROR(logger, msg) logger.Log(msg, "ERROR", __FILE__, __func__, __LINE__)

#define LOG_CUSTOM(logger, msg, level) logger.Log(msg, level, __FILE__, __func__, __LINE__)