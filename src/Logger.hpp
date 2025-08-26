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

    void log(const std::string &message);
};
