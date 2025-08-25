#pragma once
#include <string>
#include <cstdint>
#include "Server.hpp"

class Logger
{
private:
    Server &m_server;
    int m_logger_id;

public:
    Logger() = delete;
    Logger(Server &server, int logger_id, const std::string &title);

    void log(const std::string &message);
};
