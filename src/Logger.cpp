#include "Logger.hpp"

Logger::Logger(Server &server, uint16_t logger_id, const std::string &title)
    : m_server(server), m_logger_id(logger_id)
{
    // Automatically register the Logger into the Server
    m_server.f_RegisterLogger(logger_id, title);
}

void Logger::Log(uint16_t line,
                 const std::string &func,
                 const std::string &file_path,
                 const std::string &level,
                 const std::string &message)
{
    // seperate string is needed as string is modified
    std::string level_formatted(level);
    // uppercase the string
    std::transform(level_formatted.begin(), level_formatted.end(), level_formatted.begin(), ::toupper);
    // level is truncated automatically in encodeLogMessage

    // find index of last "/" (macOS or Linux) or "\" (Windows) in `file_path`
    size_t pos = file_path.find_last_of("/\\");
    // if no index found that give full path else get all string from (pos + 1) to path end
    // (pos + 1) since / or \ exists at index
    std::string filename = (pos == std::string::npos) ? file_path : file_path.substr(pos + 1);

    // send tha parameters to Server::encodeLogMessage to encode message
    m_server.f_Log(Server::encodeLogMessage(m_logger_id, line, func, filename, level_formatted, message));
}
