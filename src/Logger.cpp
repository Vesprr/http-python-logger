#include "Logger.hpp"

Logger::Logger(Server &server, const std::string &title)
    : m_server(server)
{
}

Logger::Logger(Server &server, uint16_t logger_id, const std::string &title)
    : m_server(server), m_logger_id(logger_id)
{
    m_server.RegisterLogger(logger_id, title);
}

void Logger::Log(const std::string &message,
                 const std::string &level,
                 const std::string &file_path,
                 const std::string &func,
                 int line)
{
    // seperate string is needed as string is modified
    std::string level_formatted(level);
    // truncate log if size is greater than 8
    if (level_formatted.size() > 8)
        level_formatted = level_formatted.substr(0, 8);
    // uppercase the string
    std::transform(level_formatted.begin(), level_formatted.end(), level_formatted.begin(), ::toupper);

    // find index of last "/" (macOS or Linux) or "\" (Windows) in `file_path`
    size_t pos = file_path.find_last_of("/\\");
    // if no index found that give full path else get all string from (pos + 1) to path end
    // (pos + 1) since / or \ exists at index
    std::string filename = (pos == std::string::npos) ? file_path : file_path.substr(pos + 1);

    // send tha parameters to Server::encodeLogMessage to encode message
    m_server.m_Log(Server::encodeLogMessage(message, m_logger_id, level_formatted, filename, func, line));
}
