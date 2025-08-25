#include "Server.hpp"

Server::Server(uint16_t port) : m_port(port) { m_Initialize(); }

Server::~Server() { Stop(); }

void Server::m_Initialize()
{
    crow::mustache::set_base("templates");

    // clang-format off
    CROW_ROUTE(appRef, "/favicon.ico")([](){
        return crow::response(204);
    });
    
    CROW_ROUTE(appRef, "/")([&](){

        crow::mustache::context ctx;
        ctx["logger_id"] = m_loggers.at(1);

        std::cout << m_loggers.at(1) << "\n";

        auto page = crow::mustache::load("server.html");
        return page.render(ctx);
    });
    // clang-format on
}

void Server::Start()
{
    appRef.port(m_port).multithreaded().run();
}

void Server::Stop() {}

void Server::RegisterLogger(int id, const std::string &title)
{
    // store logger title
    m_loggers[id] = title;

    std::cout << "Registered Logger: [" << std::to_string(id) << "] = " << title << "\n";
}

void Server::Log(const std::string &message, int id) {}

void Server::m_SendMessage(const std::string &message, int id) {}