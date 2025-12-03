//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "server/Server.hpp"

#define DEFAULT_IP "0.0.0.0"
#define DEFAULT_PORT 8080

int main() {
    Config config;

    auto ip_addr = config.get_value<std::string>("ip").value_or(DEFAULT_IP);
    unsigned short port = config.get_value<std::uint16_t>("port").value_or(DEFAULT_PORT);

    Server server(ip_addr, port, config);
    server.start();
}