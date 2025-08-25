#include "Logger.hpp"

Logger::Logger(Server &server, uint8_t logger_id, const std::string &title)
    : m_server(server), m_logger_id(logger_id)
{
    m_server.RegisterLogger(logger_id, title);
}

void Logger::log(const std::string &message)
{
    m_server.Log(message, m_logger_id);
}
