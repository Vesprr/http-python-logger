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
    Logger(Server &server, uint16_t logger_id, const std::string &title);

    void Log(uint16_t line,
             const std::string &func_name,
             const std::string &file_path,
             const std::string &level,
             const std::string &message);
};

// macros; defined so that user does not have to input line_number, function_name and file_name manually

/// For Info Log
#define LOG_INFO(logger, msg) logger.Log(__LINE__, __func__, __FILE__, "INFO", msg)
/// For Warning Log
#define LOG_WARN(logger, msg) logger.Log(__LINE__, __func__, __FILE__, "WARN", msg)
/// For Error Log
#define LOG_ERROR(logger, msg) logger.Log(__LINE__, __func__, __FILE__, "ERROR", msg)
/// For `Custom Level` Log
#define LOG_CUSTOM(logger, msg, level) logger.Log(__LINE__, __func__, __FILE__, level, msg)
